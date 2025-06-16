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




/**
	General purpose exception.
	It will automatically include the call stack trace in the error message.
	Very usefull to control what type of exceptions are thrown.
*/
public class KExceptionClass extends Exception {

	// uses
        public String message;     
        public String longMessage;     
        public String previousMessage;

  
	// has
          

        // ---------------------------------------------------------------------------------        
        
//        No....  because then people do not use the other one and put the base exception. Pass null if no base exception.
//        public KExceptionClass( String messageParam ){
//            
//            this( messageParam, null );
//        }
        
        
	/** Creates new KExceptionClass */
	public KExceptionClass( String messageParam, Exception previousLayerErrorParam ){
            
		super( messageParam );		
                
                // uses     
                message = messageParam;
                longMessage = "  ***EXCEPTION*** " + KMetaUtilsClass.getStackTrace( this ) + "  ";
                               
                // has
                
                //-------------------------------------------------------------------------------
                
                 if( previousLayerErrorParam != null ){
                     
                 
                       if( previousLayerErrorParam instanceof KExceptionClass ){

                            message = message + "\n -->" + ((KExceptionClass)previousLayerErrorParam).message ;                                                        
                            
                            previousMessage = ((KExceptionClass)previousLayerErrorParam).message;       
                            
                       }else{
                           
                            message = message + "\n -->" + previousLayerErrorParam.toString();
                            
                            previousMessage = previousLayerErrorParam.toString();     
                       }
                                                                                                                
                
                // ----------------------------------------------------------------------
                
                       
                       if( previousLayerErrorParam instanceof KExceptionClass ){

                            longMessage = longMessage + "\n -->" + ((KExceptionClass)previousLayerErrorParam).longMessage ;
                            
                                                                       
                       }else{
                           
                            longMessage = longMessage + "\n -->" + KMetaUtilsClass.getStackTrace( previousLayerErrorParam );
                            
                          
                       }                                                                                                           
                
                //------------------------------------------------------------------------------
                                        
                 }
                 
                //System.out.println( longMessage );
                 
                 
                
         }
        
        
        // ---------------------------------------------------------------------------------        

	/** Shows error message */
	public String getLongMessage(){
        
		return( longMessage );
	}

        // ---------------------------------------------------------------------------------
        
        
        @Override
        public String toString(){
            
            return( message );
        }
        
        // ---------------------------------------------------------------------------------        
}

