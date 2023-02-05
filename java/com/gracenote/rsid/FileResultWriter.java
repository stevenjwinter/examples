package com.gracenote.rsid;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.logging.Logger;

public class FileResultWriter implements ResultWriter {

    static Logger log = Logger.getLogger(App.class.getName());

    private String filename;

    public FileResultWriter(String filename){
        this.filename = filename;

    }

    @Override
    public void write(List<WebQueryResult> results) throws IOException {
        DateFormat dateFormat = new SimpleDateFormat("_yyyyMMddHH24:mm:ss");
        Date date = new Date();
        String prefix = dateFormat.format(date);
        File file = new File(this.filename + prefix);
        if (file.createNewFile())
        {
            log.info("Result file created");
        }


        FileWriter writer = new FileWriter(file);
        for (WebQueryResult w : results) {
            writer.write(w.getQueryName() + '\t' + w.getQueryParams() + '\t' + w.getElapsedTime()  + '\t' + w.getStationCount() + '\t' + w.getStatusCode() + "\n");
        }

        writer.close();
    }
}
