/*
This source code is part of the KFRAMEWORK  (http://k-framework.sourceforge.net/)
Copyright (C) 2001  Alejandro Vazquez, Ke Li
Feedback / Bug Reports avazqueznj@users.sourceforge.net

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



package KFramework30.FieldValidators;

// rtl
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.*;
import javax.swing.text.*; 
import java.awt.Toolkit;
import java.awt.*;
import java.net.*;
import java.io.*;
import java.util.*;

import KFramework30.Base.*;

public class numbersFieldValidatorClass 
extends DefaultStyledDocument {

	// uses
	int maxNumbers;
        String fieldName;
        boolean showCommas = false;

	// has - defaulted

        /** Creates new limitedNumbersDocumentClass */
	public numbersFieldValidatorClass( int maxNumbersParam, String fieldNameParam, boolean showCommasParam ){
	
		// uses
        	maxNumbers = maxNumbersParam;
                fieldName = fieldNameParam;
                showCommas = showCommasParam;
				
		// has - default
	}


        @Override
	public void insertString(
		int offset, String data, AttributeSet attributeSet) 
	        throws BadLocationException {
        
                
                String result  = "";
        
                for( char character: data.toCharArray() ){

                    if( 

                        ( !Character.isDigit( character ) ) &&                            
                        ( character != '.' ) &&
                        ( character != ',' ) &&                        
                        ( character != '(' ) &&
                        ( character != ')' ) &&                                                                                                            
                        ( character != '-' ) &&
                        ( character != ' ' ) &&
                        ( character != 'E' )                      
                            
                       )
                    {                        
                            Toolkit.getDefaultToolkit().beep();

                            if( data.length() > 1 ){ 
                                
                                KMetaUtilsClass.showErrorMessageFromText1( 
                                        null,
                                        "*** Could not insert data ***" +
                                        " Invalid character in number data. [" + character + "]" );                                                

                                return;                                                                
                            }

                    }else{
                    
                        result += character;
                    }
                                                                        
                }  //result
                    
                
            //-----------------------------------------
        
                if( showCommas == false ){
                    
                    result = "";
                
                    for( char character: data.toCharArray() ){

                        if( !Character.isDigit( character ) ) {                        
                                                        
                                Toolkit.getDefaultToolkit().beep();

                                if( data.length() > 1 ){ 

                                    KMetaUtilsClass.showErrorMessageFromText1( 
                                            null,
                                            "*** Could not insert data ***" +
                                            " Invalid character in number data. [" + character + "]" );                                                

                                    return;                                                                
                                }

                        }else{

                            result += character;
                        }

                    }  //result
                    
                } // show commas
                                
        
                
            //-------------------------------------------------------
            // check size
       
                String fullStringToInsert = this.getText(0, getLength()) + result;                                                                        
		if( fullStringToInsert.length() > maxNumbers){
		
			if( data.length() == 1 ){
			
				// if typed
				Toolkit.getDefaultToolkit().beep();
				
			} else{

				// if inserted
				KMetaUtilsClass.showErrorMessageFromText1( 
                                        null,
					"*** Could not insert data in " + fieldName + maxNumbers + " ***" +
					" Text is too long for number field."
				);
                                                                
			}
			
                        return;
    		}                   
                
                //-----------------------------------------------------------------------
                
                              
                super.insertString( offset, result, attributeSet );                               
	}

} //class



