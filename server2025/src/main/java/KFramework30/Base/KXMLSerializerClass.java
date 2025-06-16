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

import com.thoughtworks.xstream.XStream;
import com.thoughtworks.xstream.io.xml.StaxDriver;


public class KXMLSerializerClass {
    
    // uses
    private KConfigurationClass configuration;
    private KLogClass Mainlog;
    
    // has
    private XStream xstream;
        
    // Interface
    public KXMLSerializerClass( KConfigurationClass configurationParam, KLogClass logParam ){
        
        // uses        
        configuration = configurationParam;
        Mainlog = logParam;    
        
        // has - defaulted
        xstream = new XStream( );
            
    }
    
    
    public String serializeObject( Object object, KLogClass log) throws KExceptionClass{   
        
        String result;
        
        try{
        
            result = xstream.toXML( object );     
            
            log.log( this,  ">>>>>>>>>>>>>>  Serializing done -> \n" + result );        
            
        }catch( Exception error ){
            
            throw new KExceptionClass( "Could not serialize object :" + error.toString(), error );
        }
        
        return( result );                    
    }
    
    public Object materializeObject( String xmlStream, KLogClass log  ) throws KExceptionClass{
                        
        log.log( this,  ">>>>>>>>>>>>>>  Materializing from -> \n" + xmlStream );
        
        Object result;
        
        try{
            
            if( xmlStream == null ){
                return( null );
            }

            if( xmlStream.equals( "" )){
                return( null );
            }
            
            result = xstream.fromXML(xmlStream);
            
        }catch( Exception error ){
            
            throw new KExceptionClass( "Could not materialize from xml :" + error.toString(), error );            
        }            
        
        return( result );
    }
    
    
}
