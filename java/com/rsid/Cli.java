package com.gracenote.rsid;


import org.apache.commons.cli.*;

public class Cli {
	private String queryFile;
	private String configFile;
	private int numQueries;
	
	private Options options = new Options();

	public Cli(String[] args) {
		  options.addOption("h", "help", false, "show help.");
		  options.addOption("q", "query-file", true, "radio station queries [REQUIRED]");
		  options.addOption("c", "config-file", true, "configuration file [REQUIRED]");
		  options.addOption("n","num-queries",true,"number of queries per second");
		  parse(args);
	}
	
	private void parse(String[] args) {
		CommandLineParser parser = new DefaultParser();
		CommandLine cmd = null;
		try {
			cmd = parser.parse(options, args);

			if (cmd.hasOption("h"))
				help();

			if (cmd.hasOption("query-file")) {
				queryFile = cmd.getOptionValue("query-file");
			}else {
				System.out.println("Invalid argument. Query file is required");
				help();
			}

			if (cmd.hasOption("config-file")) {
				configFile = cmd.getOptionValue("config-file");
			} else {
				System.out.println("Invalid argument. Config file is required");
				help();
			}

			if (cmd.hasOption("num-queries")) {
				numQueries = Integer.parseInt(cmd.getOptionValue("num-queries"));
			}
				
		}

		catch (ParseException e) {
			help();
		}
	}
	
	private void help() {
		  HelpFormatter formater = new HelpFormatter();
		  formater.printHelp("Main", options);
		  System.exit(0);
	}

	public String getQueryFile() {
		return queryFile;
	}

	public void setQueryFile(String queryFile) {
		this.queryFile = queryFile;
	}

	public String getConfigFile() {
		return configFile;
	}

	public void setConfigFile(String configFile) {
		this.configFile = configFile;
	}


	public int getNumQueries() {
		return numQueries;
	}

	public void setNumQueries(int numQueries) {
		this.numQueries = numQueries;
	}
}
