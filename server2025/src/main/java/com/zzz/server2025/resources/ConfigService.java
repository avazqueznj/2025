package com.zzz.server2025.resources;

import KFramework30.Base.KMetaUtilsClass;
import KFramework30.Base.persistentTextClass;
import java.util.logging.Logger;
import java.io.IOException;
import java.io.PrintWriter;
import jakarta.servlet.ServletException;
import jakarta.servlet.annotation.WebServlet;
import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import java.io.BufferedReader;
import java.util.Iterator;

@WebServlet(name = "config", urlPatterns = {"/config"})
public class ConfigService extends HttpServlet {
    
    
    @Override
    protected void doGet(
            HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {

              
        Logger logger = Logger.getLogger(getClass().getName());
        logger.info("GETCONFIG: Start....12");
                
        try{                                  
            logger.info( "GETCONFIG: Sending config...." + KMetaUtilsClass.timeStamp() );
            
            logger.info("GETCONFIG: Loading....");            
            persistentTextClass configFile = new persistentTextClass( "EVIRconfig1.txt" );
            configFile.loadFromDisk();                        
            logger.info("GETCONFIG: Loading.... DONE");
            
            response.setContentType("text/html;charset=UTF-8");
            PrintWriter writer = response.getWriter();
                                  
            //write the response
            writer.println( "DATE*" + KMetaUtilsClass.timeStamp() );
            Iterator configFileLine = configFile.iterator();
            while( configFileLine.hasNext() ){
                writer.println( configFileLine.next() );
            }

            writer.close();
            logger.info( "GETCONFIG: Send.... DONE!\r\n");            
                                               
        }catch( Exception error ){
            logger.severe(                    
                    KMetaUtilsClass.getStackTrace(error) );
        }
                        
    }
    
    //------------------------------------------------
            
}

