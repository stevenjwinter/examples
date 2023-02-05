package com.gracenote.rsid;

import java.util.*;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.stream.Collectors;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

import javax.mail.Session;

public class App {

    static Logger log = Logger.getLogger(App.class.getName());
    public static void main (String[] args) throws Exception {

        PropertyConfigurator.configure("log4j.properties");

        // Get the command line arguments

        Cli commandLine = new Cli(args);

        if (commandLine.getConfigFile() == null){
            throw new RuntimeException("No config file");
        }

        Configuration config = new Configuration(commandLine.getConfigFile(),commandLine);
        log.info("-Configuration-");
        log.info("Query file: " + config.getQueryFile());
        log.info("Number of calls: " + config.getTotalNumOfQueries());

        String accessToken = WebQuery.getToken(config.getTokenUrl(), config.getApiCredential());

        log.info("ACCESS TOKEN: " + accessToken);
        // Get the api query parameters
        QueryReader queryReader = new FileQueryReader(config.getQueryFile());
        Map<String,Map<String,String>> queries = queryReader.getAllQueries();

        log.info("Reading query from the query file");
        // Make a list of all queries
        List<WebQuery> webQueries = getWebQueries(config.getWebApiUrl(), queries, accessToken, config.getApiCredential());

        // Execute concurrent queries
        int numThreads = config.getTotalNumOfQueries()/(config.getTotalTime());
        ExecutorService executor = Executors.newFixedThreadPool(numThreads);

        List<Future<WebQueryResult>> queryTasks = new ArrayList<Future<WebQueryResult>>();

        int counter = 0;
        log.info("Executing query in " + numThreads + " threads");
        while (counter < config.getTotalNumOfQueries()) {

            for (WebQuery q : webQueries) {
                queryTasks.add(executor.submit(q));
                counter++;
            }
        }

        executor.shutdown();

        List<WebQueryResult> queryResults = new ArrayList<>();

        // Collect the results
        for (Future<WebQueryResult> task: queryTasks){
            try {

                WebQueryResult result = task.get();
                queryResults.add(result);

            } catch (ExecutionException e) {
                throw new RuntimeException(e);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        log.info("Query execution complete");

        // Write to file
        //ResultWriter fileResultWriter = new FileResultWriter(config.getOutputFile());
        //fileResultWriter.write(queryResults);


        // Write the log
        for (WebQueryResult w : queryResults){
            System.out.println(w.getQueryName() + '\t' + w.getQueryParams() + '\t' + w.getElapsedTime()  + '\t' + w.getStationCount());
        }

        log.info("Result writing complete");
        log.info("Validating results");

        List<WebQueryResult> validationResult = validateResult(queryResults, config.getLatencyThresholdInSec());


        if (validationResult.size() > 0){
            String emailBody = createHtmlEmailBody(config.getTotalNumOfQueries(), validationResult);
            log.info("---------------------------------------------");
            log.info("WARNING - Some queries were not successful");
            log.info("---------------------------------------------");

            for (WebQueryResult r: validationResult){
                log.info(r.getQueryName() + "\t" + r.getQueryParams() + "\t" + r.getElapsedTime() + "\t" + r.getStatusCode() + "\t" + r.getFailureReason() + "\n");
            }

            sendEmail(config.getEmailRecipient(), emailBody);

        } else{
            log.info("---------------------------");
            log.info("All queries we successful");
            log.info("---------------------------");
        }
    }

    private static String createHtmlEmailBody(int numQueries, List<WebQueryResult> validationResult) {
        String emailBody = "<h3> WARNING: Some queries did not succeed </h3>";

        emailBody += "<h3> Total queries - " + numQueries +  "</h3>";

        String tableHeader = "<table style=\"width:100%\">";
        tableHeader += "<tr bgcolor=\"#ADD8E6\">";
        tableHeader += "<th>Query Name</th>";
        tableHeader += "<th>Query Params</th>";
        tableHeader += "<th>Latency</th>";
        tableHeader += "<th>Status Code</th>";
        tableHeader += "<th>Reason</th>";
        tableHeader += "</tr>";

        emailBody += tableHeader;

        for (WebQueryResult s : validationResult){
            String row = "<tr>";
            row += "<td>" + s.getQueryName() + "</td>";
            row += "<td>" + s.getQueryParams() + "</td>";
            row += "<td>" + s.getElapsedTime() + "</td>";
            row += "<td>" + s.getStatusCode() + "</td>";
            row += "<td>" + s.getFailureReason() + "</td>";
            row += "</tr>";

            emailBody += row;
        }
        emailBody += "</table>";

        return emailBody;
    }

    private static void sendEmail(String emailRecipient, String emailBody) {
        String smtpHostServer = "mailhost.globix-sc.gracenote.com";

        Properties props = System.getProperties();

        props.put("mail.smtp.host", smtpHostServer);

        Session session = Session.getInstance(props, null);

        EmailNotification.sendEmail(session, emailRecipient,"WARNING - RSID Web API Monitor Program", emailBody);
    }

    private static List<WebQueryResult> validateResult(List<WebQueryResult> results, int latencyTheshold){

        List<WebQueryResult> validationResults = new ArrayList<>();

        List<WebQueryResult> unsuccessfulQueries = results.stream()
                .filter( x -> x.getStatusCode() != 200)
                .collect(Collectors.toList());

        for (WebQueryResult w : unsuccessfulQueries){
            WebQueryResult validationResult = w;
            validationResult.setFailureReason("Status Code NOT 200");
            validationResults.add(validationResult);
        }

        List<WebQueryResult> longQueries = results.stream()
                .filter( x -> x.getElapsedTime() > 1000*latencyTheshold)
                .collect(Collectors.toList());

        for (WebQueryResult l : longQueries){
            WebQueryResult validationResult = l;
            validationResult.setFailureReason("Latency greater than " + latencyTheshold);
            validationResults.add(validationResult);
        }
        return validationResults;

    }

    private static List<WebQuery> getWebQueries(String webUrl, Map<String, Map<String, String>> queries, String accessToken, String credential) {

        List<WebQuery> webQuery = new ArrayList<WebQuery>();
        for (String qName : queries.keySet()){
            WebQuery wq = new WebQuery(webUrl,qName,queries.get(qName), accessToken);
            webQuery.add(wq);
        }

        return webQuery;
    }

}
