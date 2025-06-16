/*
This source code is part of the KFRAMEWORK  (http://k-framework.sourceforge.net/)
Copyright (C) 2001  Alejandro Vazquez, Ke Li
Feedback / Bug Reports vmaxxed@users.sourceforge.net

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

package KFramework30.Base;

import java.net.*;
import java.util.jar.*;
import java.awt.*;
import java.lang.*;
import java.io.*;
import java.util.*;
import java.util.zip.*;


/**
	General purpose class that loads JAR files from URLS.
	It can return images from the jar.
*/
public final class KJarResourcesClass {

    // uses 
    private KConfigurationClass configuration;
    private KLogClass log;
    
    // has - defaulted
    public boolean debugOn=false;   
    private Hashtable htSizes = new Hashtable();  
    private Hashtable htJarContents = new Hashtable();
    private String jarFileName;

    
    // -------------------------------------------------------------------------
    // INTERFACE


    /** Constructor */
    public KJarResourcesClass( KConfigurationClass configurationParam, KLogClass logParam ){

	// uses
	configuration = configurationParam;
	log = logParam;

	// has


	log.log( this, "Created successfully" );
    }

    /** Get a blob from jar */
    public byte[] getBytes(String name) 
    throws Exception{
    
        byte[] result = ( byte[] ) htJarContents.get( name );
    
        if( result == null ){
            
            String message = "[";
            message += name;
            message += "] not found in table.";        
            
            throw new Exception( message );
        };
    
        return result;    
    }
    
    //-----------------------------------------------------------------------   
    
    /** Gets an image from a jar */
    public Image getImage( String imageName )
    throws KExceptionClass{
    
	// report
	{
		String message = "Reading image [";
		message += imageName;
		message += "] from JAR file [";
		message += jarFileName;
		message += "]...";	
		log.log( this, message );
	}

        Image image;
    
        try{                    

            // get image
            image = Toolkit.getDefaultToolkit().createImage( getBytes( imageName ) );		
			
        }catch( Exception error ){                    
            
            try{
                            
                // not found show error image
                image = Toolkit.getDefaultToolkit().createImage( getBytes( "error.gif" ) );		
                            
            }catch( Exception againError ){

		String message = "Error creating image [";
		message += imageName;
		message += "]";		
		message += " from resource file [";
		message += jarFileName;
		message += "]";		
		throw new KExceptionClass( message, new KExceptionClass( againError.toString(), null ) );
            }
        }
                
        return( image );			
    }

    //-----------------------------------------------------------------------
       
    public void loadJarFile( String jarFileNameParam )
    throws KExceptionClass{
   
        jarFileName = jarFileNameParam;

	// report
	{
		String message = "Loading resource file [";
		message += jarFileName;
		message += "]...";	
		log.log( this, message );
	}
  
        try {
          
            // open the reource
            URL url = new URL( jarFileName );       	           	        
            JarURLConnection jarConnection = ( JarURLConnection ) url.openConnection();       	      	            	
        
            // read from connection
            jarConnection.setUseCaches( false );
            JarFile jarFile = jarConnection.getJarFile();      	    
            
            // enumerate entries
            Enumeration jarEntries = jarFile.entries();
          
            // read sizes and names
            while( jarEntries.hasMoreElements() ){
          
                // load zip entrie
                ZipEntry zipEntrie = ( ZipEntry ) jarEntries.nextElement();              

		// report
		{
			String message = "Scanning [";
			message += jarFileName;
			message += "] found [";	
			message += describeEntry( zipEntrie );
			message += "]";
			log.log( this, message );
		}
              
                // save to sizes
                htSizes.put( zipEntrie.getName(),  new Integer( ( int ) zipEntrie.getSize() ) );

            }; // while
          
            // done reading sizes and names
            jarFile.close();
            
            // extract resources and put them into the hashtable.
            BufferedInputStream inputBuffer = new BufferedInputStream(
                jarConnection.getJarFileURL().openStream()
            	);          
            
            // read data
            ZipInputStream input = new ZipInputStream( inputBuffer );          
            ZipEntry zipEntrie = null;
            while( ( zipEntrie = input.getNextEntry() ) != null ){
            
                // ignore directories
                if ( zipEntrie.isDirectory() ) continue;

		// report
		{
			String message = "Scanning [";
			message += jarFileName;
			message += "] loading [";	
			message += zipEntrie.getName();
			message += "] for [";
			message += zipEntrie.getSize();
			message += "] bytes.";
			log.log( this, message );
		}

             
                // read data
                int size = ( int ) zipEntrie.getSize();                          
                if( size == -1 ){
                    size = ( (Integer) htSizes.get( zipEntrie.getName() ) ).intValue();
                };
                
                // alloc mem for entrie
                byte[] entrieData = new byte[ (int)size ];
                int offset = 0;
                int dataRead = 0;
            
                // read from stream
                while( ( (int)size - offset ) > 0) {
             
                     dataRead = input.read( entrieData , offset , ( int ) size - offset );                 
                     if( dataRead == -1 ) break;
                     
                    offset += dataRead;
                } // loop
             
                // add to internal resource hashtable
                htJarContents.put( zipEntrie.getName(), entrieData );
                
		// debugo------------------------------------------
                if( debugOn ){
                    System.out.println(
                        zipEntrie.getName()+"  offset="+ offset +
                        ",size="+size+
                        ",csize="+ zipEntrie.getCompressedSize()
                    );
                };
		// debugo------------------------------------------

            }; // while
                
        }catch( Exception error ){

		String message = "Error loading data from JAR file [";
		message += error.toString();
		message += "]";		
		throw new KExceptionClass( message, new KExceptionClass(  error.toString(), null  ) );
	};

    }

   
    //-----------------------------------------------------------------------   

   
    // get description of entries
    private String describeEntry( ZipEntry entry ){
       
        StringBuffer message = new StringBuffer();
       
        if( entry.isDirectory() ){

            message.append( "d " ); 
        }else{

            message.append( "f " ); 
        };

       if( entry.getMethod() == ZipEntry.STORED ){

            message.append( "stored   " ); 
       } else {

            message.append( "defalted " );
       };

       message.append( entry.getName() );
       message.append( "\t" );
       message.append( "" + entry.getSize() );

       if( entry.getMethod() == ZipEntry.DEFLATED ){
            message.append( "/" + entry.getCompressedSize() );
       }

       return( message.toString() );
   }

    //-----------------------------------------------------------------------
 
}

