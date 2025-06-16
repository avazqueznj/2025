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

// rtl
import java.util.*;
import java.io.*;
import java.applet.*;


/**
	General purpose class that creates, loads and saves configuration files
	The file can be loaded from an URL, or local disk.
	You can not create, or write to URLs.
	The class provides methods to read the keys, and write key-value pairs.
*/
public class KConfigurationClass extends persistentTextClass {

	// uses
        public static AppletContext       appletContext;

	// has - defaulted
	public Properties configurationData;

	//----------------------------------------------------------------------
	// INTERFACE

        
	// -------------------------------------------------------------------------
        
        public static void openHelp( String subject )
        throws KExceptionClass
        {
            
            try {

                appletContext.showDocument( new java.net.URL( "http://userManual.pdf" ) );

            }
            catch( Exception error ){

                throw new KExceptionClass( error.toString(), error );
            }                                
        
        
        }
        
	// -------------------------------------------------------------------------        
        

	/** Creates new KConfigurationClass */
	public KConfigurationClass(){
		super();

		// uses

		// has
		configurationData = new Properties();
	}	

	/** Creates new KConfigurationClass */
	public KConfigurationClass( String fileNameParam ){
		super( fileNameParam );

		// uses

		// has
		configurationData = new Properties();
	}	

	//----------------------------------------------------------------------

	/** Builds the configuration data container from the flat data  */
	public void parse()
	throws KExceptionClass{

		// buffer to save all strings
		StringWriter buffer = new StringWriter();
		// buffered writer
		BufferedWriter bufferWriter = new BufferedWriter( buffer );

		// save contents to buffer
		ListIterator rows = listIterator();
		while( rows.hasNext() ){

			// get next row
			String currentRow = ( String ) rows.next();

			try{

				// add to buffer
				bufferWriter.write( currentRow );
				// add break
				bufferWriter.newLine();
					
				bufferWriter.flush();
					
			}catch( IOException error ){

				String message = new String();
				message = "Could not buffer file [";
				message += fileName;
				message += "][";
				message += error.toString();
				message += "]";
				throw new KExceptionClass( message, error );				
			};

		};
                
                try{
	            if( bufferWriter != null )
			bufferWriter.close();

		}catch( Exception closeError ){};                
                              
		try{
			// Load an parse data into configuration 
			configurationData.load( 
				new BufferedInputStream( 
					new ByteArrayInputStream( 
						buffer.toString().getBytes() 
					) 
				)
			);

		}catch( IOException error ){

			String message = new String();
			message = "Could not buffer file [";
			message += fileName;
			message += "][";
			message += error.toString();
			message += "]";
			throw new KExceptionClass( message, error );				
		};
                
                try{
	            if( buffer != null )
			buffer.close();

		}catch( Exception closeError ){};                     

		// done.
	}

	//----------------------------------------------------------------------	

	public String getField( String fieldName )
	throws KExceptionClass{

		// get field data
		String result = configurationData.getProperty( fieldName );
		if( result == null ){

			String message = new String();
			message = "Field [";
			message += fieldName;
			message += "] not found in file [";
			message += fileName;
			message += "]";
			throw new KExceptionClass( message, null );				
		};

		return( result );
	}

	//----------------------------------------------------------------------	

	public long getLongField( String fieldName )
	throws KExceptionClass{            
            
            long result = -1;
            
            try{
                
		result = Long.parseLong( getField( fieldName ) );
                
            }catch( Exception error ){                
                
			String message = new String();
			message = "*** Invalid number (";
                        message += getField( fieldName );
                        message += ") ***\n";
                        message += " for field [";
			message += fieldName;
			message += "] in file [";
			message += fileName;
			message += "]\n";
			throw new KExceptionClass( message, error );				                
            };
            
            return( result );
	}
        
	public int getIntField( String fieldName )
	throws KExceptionClass{            
            
            int result = -1;
            
            try{
                
		result = new Integer( getField( fieldName ) );
                
            }catch( Exception error ){                
                
			String message = new String();
			message = "*** Invalid number (";
                        message += getField( fieldName );
                        message += ") ***\n";
                        message += " for field [";
			message += fieldName;
			message += "] in file [";
			message += fileName;
			message += "]\n";
			throw new KExceptionClass( message, error );				                
            };
            
            return( result );
	}

        public double getDoubleField( String fieldName )
	throws KExceptionClass{            

            double result = -1;
            
            try{
                
		result = Double.parseDouble( getField( fieldName ) );
                
            }catch( Exception error ){                
                
			String message = new String();
			message = "*** Invalid number (";
                        message += getField( fieldName );
                        message += ") ***\n";
                        message += " for field [";
			message += fieldName;
			message += "] in file [";
			message += fileName;
			message += "]\n";
			throw new KExceptionClass( message, error );				                
            };
            
            return( result );
                
	}
        
	//----------------------------------------------------------------------	

	public void setField( String fieldNameParam, String fieldValueParam ){

		configurationData.setProperty( fieldNameParam, fieldValueParam );
	}

	//----------------------------------------------------------------------	

	public void update(){

		// pending...
	}

	
	//----------------------------------------------------------------------	
}
