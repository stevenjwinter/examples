package com.gracenote.rsid;

import com.esotericsoftware.yamlbeans.YamlException;
import com.esotericsoftware.yamlbeans.YamlReader;

import java.io.FileNotFoundException;
import java.io.FileReader;
import java.util.Map;

public class Configuration {
	private String webApiUrl;
	private String tokenUrl;
	private int totalNumOfQueries;
	private int totalTime;
	private String apiCredential;
	private String configFile;
	private String queryFile;
	private String elasticServer;
	private String elasticIndex;
	private String outputFile;
	private int latencyThresholdInSec;
	private String emailRecipient;
	// constructor is set as private so that the class cannot be instantiated from outside
	public Configuration(String configFile, Cli commandLineArgs){
		this.configFile = configFile;
		loadConfig(configFile);
		overrideWithCommandLine(commandLineArgs);
	}
	
	private void overrideWithCommandLine(Cli cli) {
		if (cli.getQueryFile() != null){
			this.queryFile = cli.getQueryFile();
		}

		if (cli.getNumQueries() != 0){
			this.totalNumOfQueries = cli.getNumQueries();
		}
	}

	// parse and load yaml config file
	private void loadConfig(String configFile) {	
		try {
			YamlReader reader;
			reader = new YamlReader(new FileReader(configFile));
			
			Object object;
			object = reader.read();
			Map map = (Map) object;
			this.webApiUrl = map.get("rsid_web_api_url").toString();
			this.tokenUrl = map.get("rsid_web_api_token_url").toString();
			this.apiCredential = map.get("base64credential").toString();
			this.totalNumOfQueries = Integer.parseInt(map.get("number_of_queries").toString());
			this.totalTime = Integer.parseInt(map.get("total_time_sec").toString());
			this.queryFile = map.get("query_file").toString();
			this.elasticServer = map.get("elastic_endpoint").toString();
			this.outputFile = map.get("output_file").toString();
			this.latencyThresholdInSec = Integer.parseInt(map.get("latency_threshold").toString());
			this.emailRecipient = map.get("email_recipient").toString();
			
		} catch (FileNotFoundException | YamlException e) {
			e.printStackTrace();
			throw new RuntimeException(e.getMessage());
		}
	}

	public String getWebApiUrl() {
		return webApiUrl;
	}
	
	public int getTotalTime() {
		return totalTime;
	}
	
	public int getTotalNumOfQueries() {
		return totalNumOfQueries;
	}
	
	public String getConfigFile() {
		return configFile;
	}
	
	public String displayConfiguration(){
		String displayString = "---------Configurations----------\n";
		displayString = displayString + " Topic Name: " + this.webApiUrl + "\n";
		displayString = displayString + " Config File: " + this.configFile + "\n";
		displayString = displayString + " Elastic Endpoint: " + this.elasticServer + "\n";
		displayString = displayString + "-----------------------------------\n";
							
		return displayString;
	}


	public String getTokenUrl() {
		return tokenUrl;
	}

	public void setTokenUrl(String tokenUrl) {
		this.tokenUrl = tokenUrl;
	}

	public String getApiCredential() {
		return apiCredential;
	}

	public void setApiCredential(String apiCredential) {
		this.apiCredential = apiCredential;
	}

	public void setTotalNumOfQueries(int totalNumOfQueries) {
		this.totalNumOfQueries = totalNumOfQueries;
	}

	public void setTotalTime(int totalTime) {
		this.totalTime = totalTime;
	}

	public String getQueryFile() {
		return queryFile;
	}

	public void setQueryFile(String queryFile) {
		this.queryFile = queryFile;
	}

	public String getElasticServer() {
		return elasticServer;
	}

	public void setElasticServer(String elasticServer) {
		this.elasticServer = elasticServer;
	}

	public String getElasticIndex() {
		return elasticIndex;
	}

	public void setElasticIndex(String elasticIndex) {
		this.elasticIndex = elasticIndex;
	}

	public String getOutputFile() {
		return outputFile;
	}

	public void setOutputFile(String outputFile) {
		this.outputFile = outputFile;
	}

	public int getLatencyThresholdInSec() {
		return latencyThresholdInSec;
	}

	public void setLatencyThresholdInSec(int latencyThresholdInSec) {
		this.latencyThresholdInSec = latencyThresholdInSec;
	}

	public String getEmailRecipient() {
		return emailRecipient;
	}

	public void setEmailRecipient(String emailRecipient) {
		this.emailRecipient = emailRecipient;
	}
}
