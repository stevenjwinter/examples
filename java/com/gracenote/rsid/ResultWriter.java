package com.gracenote.rsid;

import java.io.IOException;
import java.util.List;

public interface ResultWriter {

    void write(List<WebQueryResult> results) throws IOException;

}
