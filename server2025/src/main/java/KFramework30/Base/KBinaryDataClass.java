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

import java.io.FileInputStream;
import java.lang.reflect.Array;
import javax.swing.JFileChooser;

public class KBinaryDataClass {

	// uses
	
	// has - defaulted
	private byte[] dataBuffer;
	private int dataBufferSize;
	private int realBufferSize; // never give away

//----------------------------------------------------------------------
// INTERFACE

// ---------------------------------------------------------------------
// PRIVATES 


	private void initialize(){
	
	// uses
	
	// has - defaulted
		// very ugly, but its for performance
		dataBuffer = null;

		dataBufferSize = 0;
		realBufferSize = 0;	
	}

	// ---------------------------------------------------------------------

	// Just to keep the compatibility with the
	// c++ version.
	private void release(){
	
		// has
		dataBuffer = null;
		dataBufferSize = 0;
		realBufferSize = 0;	
	}

	// ---------------------------------------------------------------------

	// releases automatically if necessary
	private void allocate( int requestedSize )
	throws KExceptionClass{

		if( requestedSize == 0 ){
			// do nothing
			dataBufferSize = 0;
			return;
		};

		// do we really need to realloc
		if( requestedSize < realBufferSize ){

			// no, not really
			dataBufferSize = requestedSize;
		}else{

			// yes, it wont fit, realloc needed

			// trash old buffer
			release();

			// dont waste time with reallocs
			if( requestedSize < 128 ){

				dataBuffer = new byte[ 255 + 1 ];
				dataBufferSize = requestedSize;
				realBufferSize = ( 255 );
			}else{

				dataBuffer = 
					new byte[ ( requestedSize * 2 ) + 1 ];
				dataBufferSize = requestedSize;
				realBufferSize = ( requestedSize * 2 );
			};


			if( dataBuffer == null ){

				String message = new String();
				message += "Could not allocate memory. [";
				message += String.valueOf( requestedSize );
				message += "] bytes requested.";				
				throw new KExceptionClass( message, null );
			};
		};
		
	}

	// ---------------------------------------------------------------------


// ---------------------------------------------------------------------	
// PUBLIC  


	/** Creates new binaryDataClass */
	public KBinaryDataClass() {
		initialize();
	}

	/** Creates new binaryDataClass */
	public KBinaryDataClass( int reserve )
	throws KExceptionClass{
		initialize();
		allocate( reserve );
	}

	/** Create new from string */
	public KBinaryDataClass( String source )
	throws KExceptionClass{
		initialize();
		copy( source.getBytes() );
	}

	/** Copy constructor */
	public KBinaryDataClass( KBinaryDataClass source )
	throws KExceptionClass{
		initialize();
		copy( source );
	}




// ---------------------------------------------------------------------
// modifiers


	// ---------------------------------------------------------------------	

	/** Empty contents */
	public void flush()
	throws KExceptionClass{ 
		allocate( 0 ); 
	}

	// ---------------------------------------------------------------------

	/** Shift left for offset */
	public void shift( int offset )
	throws KExceptionClass{

		if( offset > dataBufferSize ){
			String message = new String();
			message += "Could not shift data past [";
			message += String.valueOf( offset );
			message += "] byte. Total size is [";				
			message += String.valueOf( dataBufferSize );
			message += "].";				
			throw new KExceptionClass( message, null );
		};

		copy( dataBuffer, offset, dataBufferSize - offset );
	}



	// ---------------------------------------------------------------------
	// raw

	/** copy raw bytes */
	public void copy( byte[] sourceData )
	throws KExceptionClass{
		// forward...
		copy( sourceData, 0, Array.getLength( sourceData ) );
	}


	/** copy raw bytes section */
	public void copy( byte[] sourceData, int offset, int sourceSize )
	throws KExceptionClass{

		if( ( offset + sourceSize ) > Array.getLength( sourceData ) ){
			String message = new String();
			message += "Could not copy bytes. Requested upto byte [";
			message += String.valueOf( offset + sourceSize  );
			message += "]. Total available was [";				
			message += String.valueOf( sourceData );
			message += "].";				
			throw new KExceptionClass( message, null );
		};

		allocate( sourceSize );

		try{
			System.arraycopy( 
				sourceData, offset, 
				dataBuffer, 0, 
				sourceSize
			);

		}catch( Exception error ){
			throw new KExceptionClass( error.toString(), error );
		};
	}


	/** append raw */
	public void append( byte[] source )
	throws KExceptionClass{
		append( source, 0, Array.getLength( source ) );
	}

	/** append raw data section */
	public void append( byte[] sourceData, int offset, int sourceSize )
	throws KExceptionClass{

		if( ( offset + sourceSize ) > Array.getLength( sourceData ) ){
			String message = new String();
			message += "Could not append bytes. Requested up to [";
			message += String.valueOf( offset + sourceSize  );
			message += "]. Total available was [";				
			message += String.valueOf( sourceData );
			message += "].";				
			throw new KExceptionClass( message, null );
		};

		// get new buffer
		KBinaryDataClass temp = 
			new KBinaryDataClass( dataBufferSize + sourceSize );

		// copy old data, if any...
		if( dataBufferSize > 0 ){
			try{
				System.arraycopy( 
					dataBuffer, 0, 
					temp.dataBuffer, 0, 
					dataBufferSize
				);
				
			}catch( Exception error ){
				throw new KExceptionClass( error.toString(), error );
			};
		};

		// append new
		try{
			System.arraycopy( 
				sourceData, offset, 
				temp.dataBuffer, dataBufferSize, 
				sourceSize
			);

		}catch( Exception error ){
			throw new KExceptionClass( error.toString(), error );
		};

		// save
		copy( temp );
	}




	// ---------------------------------------------------------------------
	// another bin

	/** copy another binary class */
	public void copy( KBinaryDataClass source )
	throws KExceptionClass{
		copy( source.dataBuffer, 0, source.dataBufferSize );
	}


	/** copy another binary class, section */
	public void copy( KBinaryDataClass source, int offset, int size )
	throws KExceptionClass{

		if( ( offset + size ) > source.dataBufferSize ){
			String message = new String();
			message += "Could not copy bytes. Requested up to [";
			message += String.valueOf( offset + size );
			message += "]. Total available was [";				
			message += String.valueOf( source.dataBufferSize );
			message += "].";				
			throw new KExceptionClass( message, null );
		};

		copy( source.dataBuffer, offset, size );
	}


	/** append another bin */
	public void append( KBinaryDataClass source )
	throws KExceptionClass{
		append( source.dataBuffer, 0, source.dataBufferSize );
	}

	/** append another bin section */
	public void append( KBinaryDataClass source, int offset, int size )
	throws KExceptionClass{

		if( ( offset + size ) > source.dataBufferSize ){
			String message = new String();
			message += "Could not append bytes. Requested up to [";
			message += String.valueOf( offset + size );
			message += "]. Total available was [";				
			message += String.valueOf( source.dataBufferSize );
			message += "].";				
			throw new KExceptionClass( message, null );
		};

		append( source.dataBuffer, offset, size );
	}


	// ---------------------------------------------------------------------
	// string

	/** Copy string */
	public void copy( String source )
	throws KExceptionClass{
		copy( source.getBytes() );
	}

	/** Copy string section */
	public void copy( String source, int offset, int size )
	throws KExceptionClass{
		copy( source.getBytes(), offset, size );
	}

	/** append string */
	public void append( String source )
	throws KExceptionClass{
		append( source.getBytes() );
	}

	/** append string section */
	public void append( String source, int offset, int size )
	throws KExceptionClass{
		append( source.getBytes(), offset, size );
	}

        
//---------------------------------------------------------------------

        public void loadFromDisk( String fileChosen ) throws KExceptionClass{
                   
              FileInputStream fileInputStream = null;

              try{	  

                       // Open file 
                       fileInputStream = new FileInputStream( fileChosen );

                       // make a buffer
                       byte[] buffer = new byte[ 4096 ];

                       // read!!
                       while( true ){                      

                            int bytesRead = fileInputStream.read( buffer );

                            if( bytesRead >0 ){

                                append( buffer, 0, bytesRead );

                            }else{

                                break;
                            }

                       }
             
              }catch( Exception error ){

                        String message = new String();
                        message = "Could not open file [";
                        message += fileChosen;
                        message += "][";
                        message += error.toString();
                        message += "]";
                        throw new KExceptionClass( message, error );
                        
              }finally{
                  
                    try{

                        if( fileInputStream != null ) fileInputStream.close();

                    }catch( Exception closeError ){};
                   
              }
            
        }
                
	
// ---------------------------------------------------------------------
// accessors

	/** Gives access to raw data */
	public byte[] data(){
            
   		// get a temp buffer of right size
		byte[] result = new byte[ dataBufferSize ];

		// write data
		System.arraycopy( dataBuffer, 0, result, 0, dataBufferSize );

		return( result );
                                
	}

	/** Returns data size */
	public int length(){
		return( dataBufferSize );
	}

	/** Return as string */
	public String asString(){

		// You can not just assign the dataBuffer,
		// because the dataBuffer size is not
		// really 'dataBufferSize'.

		// get a temp
		byte[] temp = new byte[ dataBufferSize ];

		// write data
		System.arraycopy( dataBuffer, 0, temp, 0, dataBufferSize );

		// translate
		String result = new String( temp );

		return( result );
	}

// ---------------------------------------------------------------------

}



