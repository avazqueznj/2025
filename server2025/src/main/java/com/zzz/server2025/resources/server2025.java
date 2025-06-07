package com.zzz.server2025.resources;

import java.util.logging.Logger;
import java.io.IOException;
import java.io.PrintWriter;
import jakarta.servlet.ServletException;
import jakarta.servlet.annotation.WebServlet;
import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;

@WebServlet(name = "config", urlPatterns = {"/config"})
public class server2025 extends HttpServlet {



    
    
    @Override
    protected void doGet(
            HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {

        Logger logger = Logger.getLogger(getClass().getName());
        logger.info("GETCONFIG: Start....");
            
        
        logger.info("GETCONFIG: Send....");
        response.setContentType("text/html;charset=UTF-8");
        try (PrintWriter out = response.getWriter()) {
            /* TODO output your page here. You may use following sample code. */
            out.println("config:2025v1");
            out.println("version:1");
            out.println("type:truck");
            out.println("zones:1");
            out.println("zone1component1=right blinker");
            out.println("zone1component2=left blinker");
            out.println("end");
            out.close();
            
            logger.info("GETCONFIG: Send.... DONE!\r\n");            
        }
        
        
    }


}

