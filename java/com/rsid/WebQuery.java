package com.gracenote.rsid;

import com.google.gson.*;

import java.io.*;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLEncoder;
import java.util.Map;
import java.util.concurrent.Callable;

/**
 * Created by rmancheri on 3/18/19.
 */
public class WebQuery implements Callable<WebQueryResult> {
    private String webUrl;
    private String queryName;
    private Map<String, String> queryParameters;
    private String accessToken;

    public WebQuery(String webUrl, String queryName, Map<String, String> queryParameters, String accessToken){
        this.webUrl = webUrl;
        this.queryName = queryName;
        this.queryParameters = queryParameters;
        this.accessToken = accessToken;
    }

    public WebQueryResult doQuery(){
        return null;
    }

    private String getToken(){
        return null;
    }

    private String getBody(){
        return null;
    }

    public WebQueryResult call() throws Exception {
        WebQueryResult result = callWebApi(queryName, webUrl,accessToken, queryParameters);
        return result;
    }

    public static WebQueryResult callWebApi(String queryName, String webUrl, String accessToken, Map<String,String> params)  {

        WebQueryResult queryResult = new WebQueryResult();
        try {
            long start = System.currentTimeMillis();
            URL url = new URL(webUrl + getParamsString(params));
            HttpURLConnection con = (HttpURLConnection) url.openConnection();
            con.setRequestMethod("GET");
            con.setRequestProperty("Content-Type", "application/json");
            con.setRequestProperty("Authorization", "Bearer " + accessToken);
            con.setDoOutput(true);


            BufferedReader in = new BufferedReader(
                    new InputStreamReader(con.getInputStream()));
            String inputLine;
            StringBuffer response = new StringBuffer();
            int responseCode = con.getResponseCode();

            while ((inputLine = in.readLine()) != null) {
                response.append(inputLine);
            }
            in.close();


            JsonParser jp = new JsonParser();
            JsonObject responseJson = (JsonObject) jp.parse(response.toString());
            JsonObject data = (JsonObject) responseJson.get("data");
            JsonElement stationCount = data.get("totalStations");

            long finish = System.currentTimeMillis();
            long timeElapsed = finish - start;


            queryResult.setQueryParams(params);
            queryResult.setQueryName(queryName);
            queryResult.setStatusCode(responseCode);
            queryResult.setElapsedTime(timeElapsed);
            queryResult.setStationCount(stationCount.getAsInt());

        } catch (IOException ex){

            queryResult.setQueryParams(params);
            queryResult.setQueryName(queryName);
            queryResult.setStatusCode(500);
            queryResult.setElapsedTime(-1);
            queryResult.setStationCount(0);
        }

        return queryResult;
    }

    public static String getToken(String tokenUrl, String base64credential) throws Exception {

        URL url = new URL(tokenUrl);
        HttpURLConnection con = (HttpURLConnection) url.openConnection();
        con.setRequestMethod("POST");
        con.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");
        con.setRequestProperty("Authorization", "Basic " + base64credential);

        String content = "grant_type=client_credentials";

        con.setDoOutput(true);
        DataOutputStream out = new DataOutputStream(con.getOutputStream());
        out.writeBytes(content);
        out.flush();
        out.close();

        int responseCode = con.getResponseCode();

        BufferedReader in = new BufferedReader(
                new InputStreamReader(con.getInputStream()));
        String inputLine;
        StringBuffer response = new StringBuffer();

        while ((inputLine = in.readLine()) != null) {
            response.append(inputLine);
        }
        in.close();

        Gson gson = new Gson();
        JsonParser jsonParser = new JsonParser();
        JsonObject jsonResponse = (JsonObject)jsonParser.parse(response.toString());

        String accessToken;
        if (responseCode == 200){

            accessToken = jsonResponse.get("access_token").getAsString();
        } else {
            throw new Exception("Unable to get the access token");
        }

        return accessToken;
    }

    public static String getParamsString(Map<String, String> params)
            throws UnsupportedEncodingException {
        StringBuilder result = new StringBuilder();

        for (Map.Entry<String, String> entry : params.entrySet()) {
            result.append(URLEncoder.encode(entry.getKey(), "UTF-8"));
            result.append("=");
            result.append(URLEncoder.encode(entry.getValue(), "UTF-8"));
            result.append("&");
        }

        String resultString = result.toString();
        return resultString.length() > 0
                ? resultString.substring(0, resultString.length() - 1)
                : resultString;
    }
}
