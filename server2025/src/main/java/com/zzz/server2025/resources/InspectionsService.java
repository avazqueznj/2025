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

@WebServlet(name = "inspections", urlPatterns = {"/inspections"})
public class InspectionsService extends HttpServlet {
    
    
    
    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {

        // Set encoding if you're expecting UTF-8 characters
        request.setCharacterEncoding("UTF-8");

        // Read the entire body
        StringBuilder sb = new StringBuilder();
        try (BufferedReader reader = request.getReader()) {
            String line;
            while ((line = reader.readLine()) != null) {
                sb.append(line);
            }
        }

        // Now you have the full request body
        String requestBody = sb.toString();

        // Example: print or return it in response
        response.setContentType("text/plain");
        response.getWriter().write("Received content:\n" + requestBody);
    }
    
    
    
}

