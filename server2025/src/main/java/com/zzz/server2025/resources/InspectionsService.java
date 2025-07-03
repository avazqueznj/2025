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
import java.net.InetAddress;
import java.util.Iterator;

@WebServlet(name = "inspections", urlPatterns = {"/inspections"})
public class InspectionsService extends HttpServlet {
    
    
    
   @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {

        Logger logger = Logger.getLogger(getClass().getName());
        logger.info("POST: Start....12");
        
        try{
                    
            request.setCharacterEncoding("UTF-8");

            StringBuilder sb = new StringBuilder();
            try (BufferedReader reader = request.getReader()) {
                String line;
                while ((line = reader.readLine()) != null) {
                    sb.append(line);
                }
            }

            String requestBody = sb.toString();
            
            persistentTextClass inspection = new persistentTextClass( "EVIRinspection.txt" );         
            String[] lines = sb.toString().split("\\r?\\n|\\r");
            for (String line : lines) {
                inspection.add(line);
            }                        
            inspection.writeToDisk();

            System.out.println("Received POST:\n" + requestBody);
            response.setContentType("text/plain");
            response.setCharacterEncoding("UTF-8");
            response.getWriter().write( 
                    InetAddress.getLocalHost().getHostName() +
                    ": Content received @ " + KMetaUtilsClass.timeStamp() 
            );
            
        }catch( Exception error ){
            logger.severe(                    
                    KMetaUtilsClass.getStackTrace(error) );            
        }        
               
    }
            
}

