package com.gracenote.rsid;

import org.yaml.snakeyaml.Yaml;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;
import java.util.logging.Logger;

public class FileQueryReader implements QueryReader {
    static Logger log = Logger.getLogger(QueryReader.class.getName());

    private static String queryFilename;

    public FileQueryReader(String queryFilename){
        this.queryFilename = queryFilename;
    }

    public Map<String, String> getQueryParams(String queryName) throws FileNotFoundException {
        return parseQueryFile(queryFilename).get(queryName);
    }

    public Map<String, Map<String, String>> getAllQueries() throws FileNotFoundException {
        return parseQueryFile(queryFilename);
    }

    public Map<String, Map<String, String>> parseQueryFile(String queryFilename) throws FileNotFoundException {

        Map<String, Map<String,String>> result = new HashMap<String, Map<String, String>>();

        // Read the yaml file
        Yaml yaml = new Yaml();
        File initialFile = new File(queryFilename);
        InputStream targetStream = new FileInputStream(initialFile);
        Map<String, Object> obj = (Map)yaml.load(targetStream);

        // Read and map the queries and its parameters
        for (String query: obj.keySet()){
            Map params = (Map) obj.get(query);
            Map<String,String> queryMap = new HashMap<String, String>();
            for (Object param : params.keySet()){
                queryMap.put(param.toString(),params.get(param).toString() );
            }
            result.put(query,queryMap);

        }
        log.info("Parsed query file successfully");
        return result;

    }
}
