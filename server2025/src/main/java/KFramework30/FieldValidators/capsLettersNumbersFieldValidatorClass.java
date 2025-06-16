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




package KFramework30.FieldValidators;


// rtl
import javax.swing.text.*; 
import java.awt.*;
import KFramework30.Base.*;


public class capsLettersNumbersFieldValidatorClass 
extends DefaultStyledDocument {

	// uses
	int maxLetters;
        String fieldName;

	// has - defaulted

	public capsLettersNumbersFieldValidatorClass( int maxLettersParam, String fieldNameParam ){
	
		// uses
        	maxLetters = maxLettersParam;
                fieldName = fieldNameParam;
		
		// has - default
	}

        @Override
	public void insertString(
		int offset, String data, AttributeSet attributeSet) 
	        throws BadLocationException {
        
                String result = "";
                                   
                data = data.toUpperCase();
                
                for( char letra : data.toCharArray() ){
        
                          // letras                     //numeros
                    if( ( letra > 64 && letra < 91 ) || ( letra > 47 && letra < 58 ) ){
                        
                        result += letra;
       
                    }
                }
                
		// will it fit the field??
                String fullStringToInsert = this.getText( 0, getLength() ) + data;                                                        
		if( fullStringToInsert.length() > maxLetters){

			if( data.length() == 1 ){
			
				// if typed
				Toolkit.getDefaultToolkit().beep();
				
			} else{
				// if inserted
				KMetaUtilsClass.showErrorMessageFromText1( 
                                        null,
					"*** Could not insert data in " + fieldName + maxLetters + " ***" +
					" Text is too long for string field ok."
				);
                                                                
			}
			
                        return;
    		}                   
                
                 

                super.insertString( offset, result, attributeSet );                               
	}

} //class




