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
import java.io.*;
import java.util.*;
import java.net.*;

// utilities


/**
	General purpose class that creates, loads and saves text files.
	The file can be loaded from an URL, or local disk.
	You can not create, or write to URLs.
*/
public class persistentTextClass 
extends LinkedList {

	// uses


	// has - defaulted
	public String fileName;
        
        public static final int     WRITEMODE_APPEND_TO_FILE = 0;
        public static final int     WRITEMODE_OVERWRITE_FILE = 1;
        


	//----------------------------------------------------------------------
	// INTERFACE

	// constructor
	public persistentTextClass(){
		super();

        	// uses
		
        	// has
        	fileName = new String();
	}

	//----------------------------------------------------------------------     
        
        
	// constructor
	public persistentTextClass( String fileNameParam ){
		super();

		// uses
		
        	// has
        	fileName = new String( fileNameParam );    
	}
    
    
	//----------------------------------------------------------------------

	/** Empty the content */
	public void flush(){

		clear();
	}



	//----------------------------------------------------------------------

	/** load full file on disk */
	public void loadFromDisk( String fileNameParam )
	throws KExceptionClass{

		// save file name
		fileName = fileNameParam;

		// get stream to file
		BufferedReader fileReader = null;
		try{		
			fileReader = new
				BufferedReader(			
					new FileReader( fileName )
				);

		}catch( FileNotFoundException error ){

			try{
				if( fileReader != null )
					fileReader.close();

			}catch( Exception closeError ){};

			String message = new String();
			message = "Could not open file [";
			message += fileName;
			message += "][";
			message += error.toString();
			message += "]";
			throw new KExceptionClass( message, error );
		};

		// read the file into memory
		try{				
			String buffer;
			while( true ){

				buffer = fileReader.readLine();				
				if( buffer == null ) break;

				// save line
				addLast( ( Object ) buffer );
			};
                        
                        fileReader.close();

		}catch( IOException error ){

			String message = new String();
			message = "Could not read from file [";
			message += fileName;
			message += "][";
			message += error.toString();
			message += "]";
			throw new KExceptionClass( message, error );

		}finally{

			try{
				if( fileReader != null )
					fileReader.close();

			}catch( Exception closeError ){};
		};			
	}


	/** load full file on disk */
	public void loadFromDisk()
	throws KExceptionClass{

		if( fileName == "" ){
			throw new KExceptionClass(
				"Can not load file, filename not assigned yet.", null
			);
		};

		loadFromDisk( fileName );
	}

	//----------------------------------------------------------------------

	public void loadFromNetwork( String fileNameParam ) throws KExceptionClass{
            
		// save file name
		fileName = fileNameParam;

		// get an url to the file
		URL fileURL;
		try{
			fileURL = new URL( fileName );

		}catch(	MalformedURLException error ){

			String message = new String();
			message = "Could not open file [";
			message += fileName;
			message += "][";
			message += error.toString();
			message += "]";
			throw new KExceptionClass( message, error );
		};
            
                loadFromNetwork( fileURL );
        }        
        
        
	/** load full file from url */
	public void loadFromNetwork( URL fileURL )
	throws KExceptionClass{


		// get stream to file
		BufferedReader fileReader = new
			BufferedReader(			
				new InputStreamReader(
					KMetaUtilsClass.openRemoteFile(
						fileURL
					)
				)
			);


		// read the file into memory
		try{				
			String buffer;
                                                
			while( true ){

				buffer = fileReader.readLine();				
                                
				if( buffer == null ) break;

				// save line
				addLast( ( Object ) buffer );
			};
                        
                        fileReader.close();

		}catch( IOException error ){

			String message = new String();
			message = "Could not read from file [";
			message += fileName;
			message += "][";
			message += error.toString();
			message += "]";
			throw new KExceptionClass( message, error );

		}finally{

			try{
				if( fileReader != null )
					fileReader.close();

			}catch( Exception closeError ){};
		};			

			
	}

	/** load full file from url */
	public void loadFromNetwork()
	throws KExceptionClass{

		if( fileName == "" ){
			throw new KExceptionClass(
				"Can not load file, filename not assigned yet.", null
			);
		};

		loadFromNetwork( fileName );
	}

//----------------------------------------------------------------------
        
	/** write full file on disk */
	public void writeToDisk(  )
	throws KExceptionClass{

		if( fileName == "" ){
			throw new KExceptionClass(
				"Can not write to file, filename not assigned yet.", null
			);
		};

		writeToDisk( fileName, WRITEMODE_APPEND_TO_FILE );
	}         


	/** write full file on disk */
	public void writeToDisk( int file_write_modeParam  )
	throws KExceptionClass{

		if( fileName == "" ){
			throw new KExceptionClass(
				"Can not write to file, filename not assigned yet.", null
			);
		};

		writeToDisk( fileName, file_write_modeParam  );
	}   
        
                      
	/** write full file on disk */
	public void writeToDisk( String fileNameParam, int file_write_mode )
	throws KExceptionClass{

		// save file name
		fileName = fileNameParam;


		// get stream to file
		BufferedWriter fileWriter = null;                
		try{	
                    
                    if( file_write_mode == WRITEMODE_OVERWRITE_FILE ){ 

			fileWriter = new BufferedWriter( new FileWriter( fileName, false ) );

                    }else if( file_write_mode == WRITEMODE_APPEND_TO_FILE ){

			fileWriter = new BufferedWriter( new FileWriter( fileName, true ) );
                        
                    }else{

                        throw new KExceptionClass( "Invalid log write mode! ", null );
                    }
                                                                                   
		}catch( IOException error ){

			String message = new String();
			message = "Could not open file [";
			message += fileName;
			message += "][";
			message += error.toString();
			message += "]";
			throw new KExceptionClass( message, error );
		};			


                // ======================================================================
                
		// write the file
		try{				

			Iterator rows = iterator();

			while( rows.hasNext() ){
                                                            
				fileWriter.write( ( String ) rows.next() );				
				
                                if( rows.hasNext() ) fileWriter.newLine();
			}

			fileWriter.flush();
			fileWriter.close();

		}catch( IOException error ){

			String message = new String();
			message = "Could not write to file [";
			message += fileName;
			message += "][";
			message += error.toString();
			message += "]";
			throw new KExceptionClass( message, error );

		}finally{

			try{
				if( fileWriter != null ) fileWriter.close();

			}catch( Exception closeError ){};
		};			
			
                
                if( file_write_mode == WRITEMODE_APPEND_TO_FILE ){     
                    
                        clear();                                                                 
                }
	}


	//----------------------------------------------------------------------        
        
    public String getFileName() {
        return fileName;
    }

    public void setFileName(String fileName) {
        this.fileName = fileName;
    }

	//----------------------------------------------------------------------

        
}
