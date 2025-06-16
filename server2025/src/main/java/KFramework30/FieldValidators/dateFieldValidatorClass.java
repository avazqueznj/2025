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

import javax.swing.text.*; 
import java.awt.Toolkit;
import java.awt.*;

import KFramework30.Base.*;

    public class dateFieldValidatorClass extends PlainDocument {

        static final int DATE_LENGTH = 18;

       /** Creates new dateTimeDocumentClass */    
        public dateFieldValidatorClass( )
        {
        }

        public void insertString(int offset, String data, AttributeSet attributeSet)
        throws BadLocationException 
        {

            // will it fit the field??
            if( ( getLength() + data.length() ) > DATE_LENGTH){
                // it will not fit		
                if( data.length() == 1 ){

                        // if typed
                        Toolkit.getDefaultToolkit().beep();

                } else{

                        // if inserted
                        KMetaUtilsClass.showErrorMessageFromText1( 
                                null,
                                "*** Could not write date ***" +
                                " Text is too long for date field." +
                                data  + "]" 
                        );
                }  

                return;
            }

            // will it has right characters??               
            for( int i=0; i<data.length(); i++ ){
                if( !Character.isLetterOrDigit(data.charAt(i)) && 
                    data.charAt(i) != ',' && 
                    data.charAt(i) != '/' &&
                    data.charAt(i) != ' ' ) {
                    Toolkit.getDefaultToolkit().beep();

                    if( data.length() > 1 )
                        KMetaUtilsClass.showErrorMessageFromText1( 
                                null,
                                "*** Could not write date ***" +
                                " Invalid character in data. [" + 
                                data  + "]"  
                                );
                    return;
                }
            }
                
            super.insertString( offset, data, attributeSet );  
        }           
}
    
