package com.gracenote.rsid;

import java.io.IOException;
import java.util.List;

/**
 * Created by rmancheri on 3/22/19.
 */
public interface ResultWriter {

    void write(List<WebQueryResult> results) throws IOException;

}
