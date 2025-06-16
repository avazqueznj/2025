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
import java.io.*;
import java.lang.reflect.Array;
import java.util.*;

public class httpTransactionClass
extends ArrayList
{

	// uses
	private KConfigurationClass configuration;
	private KLogClass log;

	// has - defaulted
	private List queryParameters;
	private List postParameters;	
	private static final int READY = 0;
	private static final int FINISHED = 1;
	private int status = READY;

//----------------------------------------------------------------------
// PRIVATE INTERFACE



//----------------------------------------------------------------------
// PUBLIC INTERFACE

	/** 	Creates new httpTransactionClass, decends from array list.
		The list will hold all the data beign read.
	 */
	public httpTransactionClass(
		KConfigurationClass configurationParam,
		KLogClass logParam
		){
		// inherits
		super();
		
		// uses
		configuration = configurationParam;
		log = logParam;

		// has - defaulted
		queryParameters = new ArrayList();
		postParameters = new ArrayList();
		status = READY;

		log.log( this, "Constructed successfully." );
	}

	//----------------------------------------------------------------------
	// add query param

	/** Add parameters to query string of HTTP request */
	public void addQueryParameter( 
		String fieldName, String fieldValue
		)
	throws KExceptionClass{

		if( status != READY ){
			throw new KExceptionClass(
				"HTTP transaction not ready while adding query parameters", null
			);
		};

		// create new parameter
		String newParameter = new String();

		// encode
		newParameter = 
			URLEncoder.encode( fieldName ) +
			"=" +
			URLEncoder.encode( fieldValue );

		// save it
		queryParameters.add( newParameter );
	}

	//----------------------------------------------------------------------
	// add post param

	/** Adds POST method fields to HTTP request. */
	public void addPostParameter( 
		String fieldName, String fieldValue
		)
	throws KExceptionClass{

		if( status != READY ){
			throw new KExceptionClass(
				"HTTP transaction not ready while adding post parameters", null
			);
		};

		// create new parameter
		String newParameter = new String();

		// encode
		newParameter = 
			URLEncoder.encode( fieldName ) +
			"=" +
			URLEncoder.encode( fieldValue );

		// save it
		postParameters.add( newParameter );
	}


	//----------------------------------------------------------------------

	/** Executes the HTTP POST transaction */
	public void sendRequestTo( String destinationAddress )
	throws KExceptionClass{
		
		// must be ready
		if( status != READY ){
			throw new KExceptionClass(
				"HTTP transaction not ready while executing transaction", null
			);
		};

		// assemble connection string
		String connectionString = new String();
		// add server
		connectionString = destinationAddress;

		// add parameters to string
		Iterator parameters = queryParameters.iterator();
		if( parameters.hasNext() ){
			connectionString = connectionString + "?";
		};
		while( parameters.hasNext() ){
			connectionString += ( String ) parameters.next();
			if( parameters.hasNext() ){
				connectionString += "&";
			};
		};


		// report string before connecting
		{
			String message = new String();
			message = "Openning: [";
			message += connectionString;
			message += "]";
			log.log( this, message );
		}

		try{
			// get an URL
			URL destinationURL = new URL( connectionString );

			// get a connection
			HttpURLConnection connection =
				( HttpURLConnection ) 
					destinationURL.openConnection();

			// set method
			connection.setRequestMethod( "POST" );
				
			// prepare to send
			connection.setDoOutput( true );
			// prepare to receive
			connection.setDoInput( true );

			//----------------------------------------------------
			// SEND DATA

			// get a writer for POST data to send
			BufferedWriter writer  
				= new BufferedWriter(
					new OutputStreamWriter(  
						connection.getOutputStream()
					)
				);

			// send POST data
			parameters = postParameters.iterator();
			while( parameters.hasNext() ){
				writer.write( ( String ) parameters.next() );
				if( parameters.hasNext() ){
					writer.write( "&" );
				};
			};
			
			// flush all at the end
			writer.flush();

			//----------------------------------------------------
			// CHECK REPLY

			String response = connection.getHeaderField( 0 );
			if( response.indexOf( "200" ) == -1 ){

				String errorMessage = new String();
				errorMessage = "Server HTTP error [";
				errorMessage += response;
				errorMessage += "]";

				throw new KExceptionClass( errorMessage, null );
			};

			//----------------------------------------------------
			// READ DATA

			// get input reader
			BufferedReader reader = new
				BufferedReader(
					new InputStreamReader(
						connection.getInputStream()
						)
					);

			// read data
			String currentLine;
			while( ( currentLine = reader.readLine() ) != null ){
				// save
				add( currentLine );
			};

			// done reading
			reader.close();	

			// done.
			log.log( this, "HTTP transaction completed successfully." );

			// invalidate
			status = FINISHED;
					
		}catch( Exception error ){

			String errorMessage = new String();
			errorMessage = "Could not connect to [";
			errorMessage += destinationAddress;
			errorMessage += "] [";
			errorMessage += error.toString();
			errorMessage += "]";

			log.log( this, KMetaUtilsClass.getStackTrace( error ) );

			throw new KExceptionClass( errorMessage, error );
		}
		
	}

	//----------------------------------------------------------------------

	/** Return complete reply as a string */
	public String content()
	throws KExceptionClass{

		if( status != FINISHED ){
			throw new KExceptionClass(
				"Could not return content. HTTPtransaction has not been executed.", null
			);
		};

		String result = new String();
		Iterator currentRow = iterator();

		while( currentRow.hasNext() ){
			result += ( String ) currentRow.next();
			result += "\n";
		};

		return( result );
	}

	//----------------------------------------------------------------------

	/** Reset object for reuse */
	/** Dont reuse it */

	//----------------------------------------------------------------------
}


