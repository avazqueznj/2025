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

import java.awt.Component;
import java.util.logging.Level;
import java.util.logging.Logger;


public class KNonVisibleBinaryWidgetClass 
extends Component
{
    
    // uses    
    private KConfigurationClass         configuration;    
    private KLogClass                   log;   
    private String                      componentName;
    
    // has
    private KBinaryDataClass            KBinaryData;
    
    // interface -------------------
    
    public KNonVisibleBinaryWidgetClass( 
            KConfigurationClass configurationParam, KLogClass logParam, 
            String componentNamneParam, int initialSize ) 
    throws KExceptionClass{
        
        try {
            // uses
            configuration = configurationParam;
            log = logParam;
            componentName = componentNamneParam;
            setName( componentName );
            
            // has - defaulted
            KBinaryData = new KBinaryDataClass( initialSize );
            
            
            
        } catch (KExceptionClass ex) {
            
            throw new KExceptionClass( "Could not create a binary data object " + ex, ex);
        }
        
    }
    

    
    // ------------------------------------------------------------------------------
    
    public void setBinaryData( byte[] data ) throws KExceptionClass{
        
        KBinaryData.copy( data );
    }

    public byte[] getBinaryData(){
        
        return KBinaryData.data();
    }
    
    public KBinaryDataClass getBinaryObject(){
        
        return KBinaryData;
    }
      
    public int binaryDatasize(){
       
        return( KBinaryData.length() );
    }
    
    // ------------------------------------------------------------------------------
                
}
