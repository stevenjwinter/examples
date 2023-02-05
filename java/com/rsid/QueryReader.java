package com.gracenote.rsid;

import java.io.FileNotFoundException;
import java.util.Map;

/**
 * Created by rmancheri on 3/18/19.
 */
public interface QueryReader {

    public Map<String,String> getQueryParams(String queryName) throws FileNotFoundException;
    public Map<String, Map<String,String>> getAllQueries() throws FileNotFoundException;

}
