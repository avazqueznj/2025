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
import java.awt.*;
import java.io.Serializable;
import javax.swing.*;
import javax.swing.text.*;
import java.lang.annotation.*;
import java.lang.reflect.*;

// aleke utils
import java.math.BigDecimal;

public abstract class KBusinessObjectClass {

        //-----------------------------------------------
        // Materialization/Serialization 
    
        
        //Constants   
        @KNoGUIbinding
        public static final int IGNORE_MISSING_FIELDS = 0;
        @KNoGUIbinding
        public static final int DONOT_IGNORE_MISSING_FIELDS = 1;        
                
        
        
        //---------------------------------------------
        // Formatos de pantalla y datos        
        @KNoGUIbinding
        protected static final int NUMERIC_TYPE = 0;
        @KNoGUIbinding
        protected static final int NUMERIC2_TYPE = 1;        
        @KNoGUIbinding
        protected static final int NUMERIC5_TYPE = 2;                
        @KNoGUIbinding
        protected static final int CURRENCY_TYPE = 5;                                          
        @KNoGUIbinding
        protected static final int NUMERIC_NOFORMAT = 6;
        //---------------------------------------------
        @KNoGUIbinding
        protected static final int DATE_TYPE = 4;                      
        @KNoGUIbinding        
        protected static final int TIMESTAMP_TYPE = 3;        
        //---------------------------------------------                
        @KNoGUIbinding
        protected static final int CAPSLETTERS_FORMAT = 7;        
        @KNoGUIbinding
        protected static final int CAPSLETTERS_NUMBERS_FORMAT = 8;       
        // Formatos de pantalla y datos                
        //---------------------------------------------
        
        
             
        //-----------------------------------------------------------------------        
        
        @KNoGUIbinding
        protected Map< String, Integer > fieldTypes = new HashMap< String, Integer >();                                          
        @KNoGUIbinding
        protected Map< String, Integer > fieldMaxSize = new HashMap< String, Integer >();                                          
        @KNoGUIbinding
        protected Vector< String > readOnlyFields = new Vector< String >();
        
        @KNoGUIbinding
        public boolean editable = true;
        @KNoGUIbinding
        public String OIDfield = new String();
        @KNoGUIbinding
        public String versionField = new String();
        @KNoGUIbinding
        public boolean versionFieldset = false;

        @KNoGUIbinding
        public HashMap repeatedFKaliasNames = new HashMap();                  
        @KNoGUIbinding
        public Vector< String > fieldNames = new Vector< String >();    
        
        //-----------------------------------------------------------------------        
        
        @Retention(value=RetentionPolicy.RUNTIME)
        @Target(value=ElementType.FIELD)
        public @interface KNoGUIbinding {}
        
        @Retention(value=RetentionPolicy.RUNTIME)
        @Target(value=ElementType.FIELD)        
        public @interface KID {}   
        
        @Retention(value=RetentionPolicy.RUNTIME)
        @Target(value=ElementType.FIELD)        
        public @interface KObjectVersion{}            
        
        
        //-----------------------------------------------------------------------        
        
        public KBusinessObjectClass()
        throws KExceptionClass{
            
            // uses NA
            
            
            // has
            
 
            try{
                
                boolean OIDset = false;               
                
                // read field data
                Field field[] = getClass().getDeclaredFields();
                for( int i = 0; i < field.length; i++){
                                                                

                    // read fields
                    if( field[ i ].getAnnotation( KNoGUIbinding.class ) == null ){
                        
                        // hibernate and open jpa rubish ....
                        if( field[ i ].getName().startsWith("_") ) continue;
                        if( field[ i ].getName().equals( "serialVersionUID" ) ) continue;      
                        if( field[ i ].getName().equals( "pcInheritedFieldCount" ) ) continue;   
                        if( field[ i ].getName().equals( "pcFieldNames" ) ) continue;   
                        if( field[ i ].getName().equals( "pcFieldTypes" ) ) continue;   
                        if( field[ i ].getName().equals( "pcFieldFlags" ) ) continue;   
                        if( field[ i ].getName().equals( "pcPCSuperclass" ) ) continue;                        
                        if( field[ i ].getName().equals( "pcStateManager" ) ) continue;   
                        if( field[ i ].getName().equals( "pcDetachedState" ) ) continue;                                                               
                        if( field[ i ].getName().startsWith( "class$" ) ) continue;   
                                                                          
                        fieldNames.add( field[ i ].getName() ); 
                        
                        //-------------------------------------------------------                        
                        
                        // set KID
                        if( field[ i ].getAnnotation( KID.class ) != null ){                        
                                            
                            if( OIDset ){ 
                                throw new KExceptionClass( "Error, more than one field set as OID for class  " + this.getClass().getName(), null );
                            }
                            
                            OIDfield = field[ i ].getName();
                            OIDset = true;
                            
                            // set numeric
                            fieldTypes.put( OIDfield, NUMERIC_NOFORMAT );
                        }
                        
                        //-------------------------------------------------------                        
                        
                        // set objectVersion
                        if( field[ i ].getAnnotation( KObjectVersion.class ) != null ){                        
                                   
                            if( versionFieldset ){ 
                                throw new KExceptionClass( "Error, more than one field set as versionID for class  " + this.getClass().getName(), null );
                            }                            
                            
                            versionField = field[ i ].getName();
                            versionFieldset = true;                            
                        }     
                        
                        //-------------------------------------------------------
                        
                    }                                        
                }
                
                if( !OIDset ){ 
                    throw new KExceptionClass( "Error, no OID set for class " + this.getClass().getName(), null );
                }
                
            }catch( Exception error ){
                
                throw new KExceptionClass( "Cant build object ", error  );
            }

                                                                
            // ------------------------
            
        }        
        
        
        // ----------------------------------------------------------------------
        
        public long getOID() throws KExceptionClass{    
            
            try{
                
                return( KMetaUtilsClass.getIntegralNumericValueFromString( getFieldasString(OIDfield), OIDfield ) ); 
                
            }catch( Exception error ){
                throw new KExceptionClass( " Could not find OID of object ", error );
            }
        };        
        
        public Iterator getFieldNamesIterator(){
            return( fieldNames.iterator() );
        }
        
        
         //-----------------------------------------------                  
         //-----------------------------------------------                  
         //-----------------------------------------------
         // Materialize - Serialize
        
        
                        // to create objects of class
                        private  void setFieldGeneric( String fieldName, Object newValue1 ) 
                        throws KExceptionClass{
                                                         
                            
                            try{  

                                boolean FOUND = false;                                
                                boolean TRANSLATED = false;
                                
                                Method[] methods = this.getClass().getDeclaredMethods();

                                for( int index = 0; index < methods.length; index++  ){                                    
                                    
                                    String methodName = methods[ index ].getName().substring( 3 );
                                    String verb = methods[ index ].getName().substring( 0, 3 );
                                    
                                    if( verb.equals( "set" ) && methodName.equalsIgnoreCase(fieldName)  ){
                                        
                                        FOUND = true;
                                        
                                        Class[] parameterTypes = methods[ index ].getParameterTypes();
                                        

                                        // --------------------------------------------------------------
                                        // BLOB
                                                                                                                           
                                        if( parameterTypes[ 0 ].equals( byte[].class ) ){ 
                                            methods[ index ].invoke( this, (byte[]) newValue1 );
                                            TRANSLATED = true;
                                        }
                                        
                                        
                                        // --------------------------------------------------------------
                                        // Strings
                                                                                                                           
                                        if( parameterTypes[ 0 ].equals( String.class ) ){ 
                                            methods[ index ].invoke( this, (String) newValue1 );
                                            TRANSLATED = true;
                                        }
                                        
                                        // --------------------------------------------------------------                                    
                                        // Integer types
                                       
                                        if( parameterTypes[ 0 ].equals( int.class ) ){
                                            methods[ index ].invoke( this, (int) KMetaUtilsClass.getIntegralNumericValueFromString( (String)newValue1, methods[ index ].getName() )    );
                                            TRANSLATED = true;
                                        }
                                                                                    
                                        if( parameterTypes[ 0 ].equals( Integer.class ) ){
                                            methods[ index ].invoke( this, new Integer(  (int) KMetaUtilsClass.getIntegralNumericValueFromString( (String)newValue1, methods[ index ].getName() )  ) );
                                            TRANSLATED = true;
                                        }
                                        
                                        if( parameterTypes[ 0 ].equals( long.class ) ){ 
                                            methods[ index ].invoke( this, (long) KMetaUtilsClass.getIntegralNumericValueFromString( (String)newValue1, methods[ index ].getName() )  );
                                            TRANSLATED = true;
                                        }
                                        
                                        if( parameterTypes[ 0 ].equals( Long.class ) ){ 
                                            methods[ index ].invoke( this, new Long( KMetaUtilsClass.getIntegralNumericValueFromString( (String)newValue1, methods[ index ].getName() )  )  );
                                            TRANSLATED = true;
                                        }
                                        
                                        
                                        //---------------                                        
                                        // Floating points...
                                        
                                        if( parameterTypes[ 0 ].equals( float.class ) ){ 
                                            methods[ index ].invoke( this,  (float) KMetaUtilsClass.getDecimalNumericValueFromString( (String)newValue1, methods[ index ].getName() )  );
                                            TRANSLATED = true;
                                        }
                                                                                                                                                                    
                                        if( parameterTypes[ 0 ].equals( Float.class ) ){ 
                                            methods[ index ].invoke( this, new Float( KMetaUtilsClass.getDecimalNumericValueFromString( (String)newValue1, methods[ index ].getName() ) ) );
                                            TRANSLATED = true;
                                        }
                                        
                                        if( parameterTypes[ 0 ].equals( double.class ) ){ 
                                            methods[ index ].invoke( this,  KMetaUtilsClass.getDecimalNumericValueFromString( (String)newValue1, methods[ index ].getName() )  ); 
                                            TRANSLATED = true;
                                        }
                                                                                    
                                        if( parameterTypes[ 0 ].equals( Double.class ) ){ 
                                            methods[ index ].invoke( this, new Double ( KMetaUtilsClass.getDecimalNumericValueFromString( (String)newValue1, methods[ index ].getName() ) ) ); 
                                            TRANSLATED = true;
                                        }
                                        
                                        // new java type                                        
                                        if( parameterTypes[ 0 ].equals( BigDecimal.class ) ){ 
                                            methods[ index ].invoke( this, new BigDecimal ( KMetaUtilsClass.getDecimalNumericValueFromString( (String)newValue1, methods[ index ].getName() ) ) ); 
                                            TRANSLATED = true;
                                        }                                        
                                        
                                        // --------------------------------------------------------------                                            
                                        // dates
                                        
                                        if( parameterTypes[ 0 ].equals( java.util.Date.class ) ){ 
                                            
                                            if( !((String)newValue1).isEmpty() ){
                                                methods[ index ].invoke( this, 
                                                        KMetaUtilsClass.stringToDate( 
                                                            KMetaUtilsClass.KDEFAULT_LONG_DATE_TIME_FORMAT, (String)newValue1  ) ); 
                                            }
                                            
                                            TRANSLATED = true;
                                            break;
                                        }
                                        
                                        // --------------------------------------------------------------                                                                                    
                                    }
                                }
                                
                                if( !FOUND ) throw new KExceptionClass( "Field not found " + fieldName, null );
                                if( !TRANSLATED ) throw new KExceptionClass( "Field type not supported for " + fieldName, null );
                               
                            }catch(  Exception error ){
                                throw new KExceptionClass( "Cannnot materialize field " + fieldName, error );
                            }                            
                        }
                        
                         //-----------------------------------------------                                          

                        public void setField(String fieldName, byte[] newValue) 
                        throws KExceptionClass
                        {
                           setFieldGeneric( fieldName,  newValue );
                        }
                        
                        
                        public void setField(String fieldName,String newValue) 
                        throws KExceptionClass
                        {
                           setFieldGeneric( fieldName,  newValue );
                        }
                        
                        
                        public void setField(String fieldName,int newValue) 
                        throws KExceptionClass
                        {
                           setFieldGeneric( fieldName, KMetaUtilsClass.toDecimalString( newValue ) );
                        }

                        public void setField(String fieldName,long newValue) 
                        throws KExceptionClass
                        {   
                           setFieldGeneric( fieldName, KMetaUtilsClass.toDecimalString( newValue ) );
                        }

                        /*** EVIL -> Force to always use double, inconsistency will slowly eat fractions... */

                       public void setField(String fieldName,float newValue) 
                       throws KExceptionClass
                       {   
                           setFieldGeneric( fieldName, KMetaUtilsClass.toDecimalString( newValue ) );
                       }
       

                       public void setField(String fieldName,double newValue) 
                       throws KExceptionClass
                       {   
                           setFieldGeneric( fieldName, KMetaUtilsClass.toDecimalString( newValue ) );
                       }        

            //-----------------------------------------------                  
            //-----------------------------------------------                  
            //-----------------------------------------------                                         

                        // to serializeObject object of class
                        public String getFieldasString( String fieldName ) throws KExceptionClass{
                                                        
                            String result = new String();
                                                                       
                            Object rawResult = getField( fieldName );

                            if( rawResult instanceof java.util.Date   ){

                                result = (  KMetaUtilsClass.dateToString( 
                                                KMetaUtilsClass.KDEFAULT_LONG_DATE_TIME_FORMAT, (Date) rawResult ) );   
                            }                              
                            else
                            if( rawResult instanceof Number  ){

                                result = KMetaUtilsClass.toDecimalString( ((Number)rawResult).doubleValue() );                                                                                          
                                
                            }else
                            {
                                if( rawResult == null ){
                                    
                                    result = new String();
                                }else{
                                    
                                    result = (  rawResult.toString() );                                                                                    
                                }
                                
                            }                           
                                                        
                            return( result );
                                                        
                        };

                
                        
                        // to serializeObject object of class
                        public Object getField( String fieldName ) throws KExceptionClass{
                            
                            boolean OK = false;
                            
                            Object result = null;
                            
                            try{  

                                Method[] methods = this.getClass().getDeclaredMethods();

                                for( int index = 0; index < methods.length; index++  ){                                    
                                    
                                    String methodName = methods[ index ].getName().substring( 3 );
                                    String verb = methods[ index ].getName().substring( 0, 3 );
                                    
                                    if( verb.equals( "get" ) && methodName.equalsIgnoreCase(fieldName)  ){
                                                                                                            
                                        result = methods[ index ].invoke( this );    
                                            
                                        //if( result == null ) result = "";                                                                             
                                       
                                        OK = true;
                                        break;
                                    }
                                }
                                
                                
                                if( !OK ) throw new KExceptionClass( "Field not found (Missing getter/setter?) [" + fieldName + "]", null );                                

                            }catch(  Exception error ){
                                throw new KExceptionClass( "Cannnot serialize field [" + fieldName + "]", error );
                            }
                            
                            return( result );
                            
                            
                        };                        
               
         
         // Materialize - Serialize
         //-----------------------------------------------                  
         //-----------------------------------------------                  
         //-----------------------------------------------
      
        
                     
    
        //-----------------------------------------------
        // MISC
                
                
      public String toFullString() 
        throws KExceptionClass
        {
            String result = new String();

            Iterator field = fieldNames.iterator();
            while( field.hasNext() ){				

                    String currentField = (String)field.next();
        
                    result += currentField;
                    result += "=";
                    result += getFieldasString( currentField );
                    result += " | ";				
                    

            }  

            return( result );
        }	 
      
      
//------------------------------------------------------------      

        public String diff( KBusinessObjectClass comparator  ) 
        throws KExceptionClass{
            
            String result = new String();
            
            Iterator field = fieldNames.iterator();
            while( field.hasNext() ){				

                    String currentField = (String)field.next();        
                    String left = getFieldasString( currentField );
                    String right = comparator.getFieldasString( currentField );
                	
                    if( left != null && right != null ){
                        if( !left.equals( right ) ){
                            result += currentField + "=" + left + "->" + right + " | ";
                        }                        
                    }
                    
                    if( left == null && right != null ){                  
                        result += currentField + "=(null)->" + right + " | ";                                             
                    }                    
            }              
            
            return( result );
        }  
        
        
//------------------------------------------------------------
        
 public boolean compareByFields( KBusinessObjectClass other    )          
        throws KExceptionClass{
            
            boolean result = true;
            
            Iterator field = fieldNames.iterator();
            while( field.hasNext() ){				

                    String currentField = (String)field.next();        
                    String left = getFieldasString( currentField );
                    String right = other.getFieldasString( currentField );
                	
                    if( left != null && right != null ){
                        if( !left.equals( right ) ){
                            result = false;
                        }                        
                    }
                    
                    if( left == null && right != null ){                  
                        result = false;
                    }                    
                    
                    if( left != null && right == null ){                  
                        result = false;
                    }                    
                    
            }              
            
            return( result );
        }
        
 //------------------------------------------------------------       
 
  public boolean compareCommonFields( KBusinessObjectClass other    )          
        throws KExceptionClass{
            
            boolean result = true;
            
            Iterator field = fieldNames.iterator();
            while( field.hasNext() ){				

                    String currentField = (String)field.next();   

                    String left = "";
                    String right = "";
                                        
                    try{
                        
                        left = getFieldasString( currentField );
                        right = other.getFieldasString( currentField );
                        
                    }catch( Exception error ){
                        
                        continue;
                    }
                	
                    if( left != null && right != null ){
                        if( !left.equals( right ) ){
                            result = false;
                        }                        
                    }
                    
                    if( left == null && right != null ){                  
                        result = false;
                    }                    
                    
                    if( left != null && right == null ){                  
                        result = false;
                    }                    
                    
            }              
            
            return( result );
        }
 
//------------------------------------------------------------                
//------------------------------------------------------------
     
  
    public void displayVisualize(  
        Container visualDisplay,      // container of destination fields
        java.util.List additionalComponentList )     // components that may not be found in the container
    throws KExceptionClass{
        
        displayVisualize( visualDisplay, additionalComponentList, DONOT_IGNORE_MISSING_FIELDS );
    }
            
                    
    public void displayVisualize(  
        Container visualDisplay,      // container of destination fields
        java.util.List additionalComponentList,     // components that may not be found in the container
        int missingFieldsBehavior  )  // Ignore fields for which there is no component?? <- for informational windows
    throws KExceptionClass
    {
        
        //auto save serial        
        if( versionFieldset ){
            JLabel versionFieldComponent = new JLabel();
            versionFieldComponent.setName( versionField );
            additionalComponentList.add( versionFieldComponent );
        }
        
        // Buffer all components
        java.util.List widgetList = new ArrayList();
        KMetaUtilsClass.getComponentsFromContainer( visualDisplay, widgetList );        
        
        widgetList.addAll( additionalComponentList );

        // iterate through all attributes of PDC object
        Iterator fields = getFieldNamesIterator();
        while( fields.hasNext() ){  

            // Buffer each attribute
            String currentField = ( String )fields.next();                      
            
            // skip versioning field
            if( currentField.equals( "object_version" ) ) continue;
            
            // get value
            String currentValue = getFieldasString( currentField );
            
            // get desired format
            Integer fieldTypeFormat = (Integer) fieldTypes.get( currentField );            
            
    // --------------------------------------------------------------------
    // format value
    // --------------------------------------------------------------------
            
            if( ( fieldTypeFormat != null ) && ( !currentValue.equals( "" ) ) ){
                                   
                    switch( fieldTypeFormat.intValue() ){

                        case NUMERIC_NOFORMAT : {                                 
                            currentValue = 
                                String.valueOf( 
                                    (long) KMetaUtilsClass.getDecimalNumericValueFromString( currentValue, currentField )
                                    );
                            break;
                        }
                        
                        
                        case NUMERIC_TYPE : {                                 
                            currentValue = 
                                KMetaUtilsClass.toDecimalString( 
                                    KMetaUtilsClass.getDecimalNumericValueFromString( currentValue, currentField ), 0 
                                    );
                            break;
                        }

                        case NUMERIC2_TYPE : {                                    
                            currentValue = 
                                KMetaUtilsClass.toDecimalString( 
                                    KMetaUtilsClass.getDecimalNumericValueFromString( currentValue, currentField ), 2
                                    );
                            break;
                        }

                        case NUMERIC5_TYPE : {                                    
                            currentValue = 
                                KMetaUtilsClass.toDecimalString( 
                                    KMetaUtilsClass.getDecimalNumericValueFromString( currentValue, currentField ), 5
                                    );
                            break;
                        }

                        case CURRENCY_TYPE : {                                    
                            if ( currentValue.isEmpty() || currentValue.equals("0") || currentValue.equals("0.0") || currentValue.equals("0.00") )
                                currentValue = "";
                            else
                                currentValue = 
                                    KMetaUtilsClass.toCurrencyString( 
                                        KMetaUtilsClass.getDecimalNumericValueFromString( currentValue, currentField )
                                        );
                            break;
                        }

                        case DATE_TYPE :{    
                            
                            if( !currentValue.equals( "" ) ){
                                
                                currentValue = KMetaUtilsClass.changeDateFormat(
                                        KMetaUtilsClass.KDEFAULT_LONG_DATE_TIME_FORMAT,
                                        KMetaUtilsClass.KDEFAULT_DATE_FORMAT,
                                        currentValue );                                       
                            }                            
                            
                            break;
                        }

                        case TIMESTAMP_TYPE :{                                    
                            break;
                        }

                        case CAPSLETTERS_FORMAT :{                                    
                            break;
                        }    
                            
                        case CAPSLETTERS_NUMBERS_FORMAT:{
                            break;
                        }

                        default:{
                            throw new KExceptionClass( 
                                " *** Cannot display object *** \n " +
                                " Field type not recognized [" + currentField +
                                "]", null
                                );
                        }                 
                        
                    }//end of switch
                    
                    
            }//end of if numeric value

            
    // --------------------------------------------------------------------
    // DISPLAY and SET VALIDATORS
    // --------------------------------------------------------------------            
                        
            //  Lookup components                                    
            // locate each attribute's corresponding visual widget
            Iterator components = widgetList.iterator();
            
            if( !components.hasNext() ) 
                throw new KExceptionClass( 
                    " *** Cannot display object *** \n " +
                    " No components found ", null );                   
                        
            boolean radioSet = false;
            while( components.hasNext() ){  
                                
                // for each component
                Object currentComponent = components.next(); 
                
                if( currentComponent instanceof Component  ) {
                    
                    String name = ( ( Component ) currentComponent ).getName();
                     
                    // match attribute <-> component
                    if( name != null ){
                                                                                                 
                            if( name.equals( currentField ) ) {   

                                        if( ( editable == false ) || ( readOnlyFields.contains( currentField )  ) ){
                                            ( ( Component ) currentComponent ).setEnabled( false );                  
                                        }
                            
                                        // KNonVisibleBinaryWidgetClass ------------------                            
                            
                                        if( currentComponent instanceof KNonVisibleBinaryWidgetClass ){  
                                            
                                            if( getField( currentField ) != null ){
                                           
                                                ((KNonVisibleBinaryWidgetClass)currentComponent).setBinaryData( (byte[])getField( currentField ) );                                                
                                            }else{
                                                
                                                ((KNonVisibleBinaryWidgetClass)currentComponent).setBinaryData( new byte[ 0 ] );                                                
                                            }
                                            
                                           
                                        }                                                                                             

                                        // JRADIO ------------------
                                        else if( currentComponent instanceof JRadioButton ){                                                                                                

                                            radioSet = true;
                                            
                                            // selected radio ??
                                            if( ( (JRadioButton)currentComponent).getText().equals( (Object) currentValue )  ){
                                                ( (JRadioButton)currentComponent ).setSelected( true );                                                                                                                                                      
                                            }else{                                
                                                ( (JRadioButton)currentComponent ).setSelected( false );                                                                                                            
                                            }


                                                                                                                       
                                        }


                                        // JCOMBO ------------------
                                        else if( currentComponent instanceof JComboBox ){

                                            ( (JComboBox)currentComponent).setSelectedItem( (Object) currentValue );


                                        }

                                        // JTEXT *------------------
                                        else if( currentComponent instanceof JTextComponent ){


                                            // SET TYPE 
                                            if( fieldTypeFormat != null ){

                                                switch( fieldTypeFormat.intValue() ){

                                                    //=====================================================================================                                                    
                                                    
                                                        case NUMERIC_NOFORMAT :{ 
                                                    
                                                            int maxSize = 60;
                                                            if( fieldMaxSize.containsKey( currentField ) ) maxSize = fieldMaxSize.get( currentField );                                                           
                                                            KMetaUtilsClass.setMaxNumbersNoFormatFieldValidator(  (JTextComponent) currentComponent, maxSize );
                                                        break;
                                                    }
                                                        
                                                    case NUMERIC_TYPE : {   

                                                            int maxSize = 60;
                                                            if( fieldMaxSize.containsKey( currentField ) ) maxSize = fieldMaxSize.get( currentField );                                                                                                                       
                                                            KMetaUtilsClass.setMaxNumbersFieldValidator(  (JTextComponent) currentComponent, maxSize );
                                                        break;
                                                    }

                                                    case NUMERIC2_TYPE : {                                    

                                                            int maxSize = 60;
                                                            if( fieldMaxSize.containsKey( currentField ) ) maxSize = fieldMaxSize.get( currentField );                                                                                                                       
                                                            KMetaUtilsClass.setMaxNumbersFieldValidator(  (JTextComponent) currentComponent, maxSize );                                            
                                                        break;

                                                    }

                                                    case NUMERIC5_TYPE : {    
                                                            
                                                            int maxSize = 60;
                                                            if( fieldMaxSize.containsKey( currentField ) ) maxSize = fieldMaxSize.get( currentField );                                                                                                                       
                                                            KMetaUtilsClass.setMaxNumbersFieldValidator(  (JTextComponent) currentComponent, maxSize );                                            
                                                        break;

                                                    }

                                                    case CURRENCY_TYPE : { 
                                                            
                                                            int maxSize = 60;
                                                            if( fieldMaxSize.containsKey( currentField ) ) maxSize = fieldMaxSize.get( currentField );                                                                                                                       
                                                            KMetaUtilsClass.setCurrencyFieldValidator(  (JTextComponent) currentComponent, maxSize );                                                    
                                                        break;

                                                    }

                                                    //-------------------------------------------------------------------------------------                                                            
                                                    //-------------------------------------------------------------------------------------                                                            
                                                    //-------------------------------------------------------------------------------------                                                            
                                                        
                                                    case DATE_TYPE :{  
                                                        // not necessary with new compo ???
                                                        break;
                                                    }

                                                    case TIMESTAMP_TYPE :{   
                                                        KMetaUtilsClass.setFieldValidator( (JTextComponent) currentComponent, KMetaUtilsClass.TIME_FIELD );
                                                        break; 
                                                    }

                                                        case CAPSLETTERS_FORMAT: {   

                                                            int maxSize = 255;

                                                            if( fieldMaxSize.containsKey( currentField ) ){
                                                                maxSize = fieldMaxSize.get( currentField );
                                                            }

                                                            KMetaUtilsClass.setCapsLettersFieldValidator( (JTextComponent) currentComponent, maxSize );

                                                        break;
                                                    }
                                                            
                                                        case CAPSLETTERS_NUMBERS_FORMAT:  {   

                                                            int maxSize = 255;

                                                            if( fieldMaxSize.containsKey( currentField ) ){
                                                                maxSize = fieldMaxSize.get( currentField );
                                                            }

                                                            KMetaUtilsClass.setCapsLettersNumbersFieldValidator( (JTextComponent) currentComponent, maxSize );

                                                        break;
                                                    }
                                                        
                                                    //=====================================================================================

                                                    default:{

                                                       throw new KExceptionClass( 
                                                            " *** Can not display object *** \n " +
                                                            " Invalid field type [" + currentField +
                                                            "]" , null
                                                            );                                                                                                                           
                                                    }                                          
                                                }

                                            } 
                                            else {

                                                // SET MAX SIZE
                                                if( fieldMaxSize.containsKey( currentField ) ){
                                                    KMetaUtilsClass.setMaxCharactersFieldValidator(
                                                            (JTextComponent) currentComponent ,
                                                            fieldMaxSize.get( currentField )
                                                            );    

                                                }

                                            }

                                            // SET VALUE                                
                                            ( (JTextComponent)currentComponent).setText( currentValue );    


                                        }                                                                                  
                                        // label ------------------
                                        else if( currentComponent instanceof JLabel ){

                                            ( (JLabel)currentComponent).setText( currentValue );                                
                                        }

                                        // Check box ------------------
                                        else if( currentComponent instanceof JCheckBox ){

                                            if( getFieldasString( name ).equals( "Y" ) )
                                                ( (JCheckBox)currentComponent).setSelected( true );                                                            
                                            else
                                                ( (JCheckBox)currentComponent).setSelected( false );   

                                        }    
                                                
                                                
                                                  
                                        else{                                

                                            throw new KExceptionClass( 
                                                " *** Can not display object *** \n " +
                                                " Invalid component ", null );                        
                                        }
                                                      
                            if( !radioSet )break;
                            
                        }   // end if name == currentField                 
                    } // end if null
                } // end if Component

                if( currentComponent instanceof KCustomWidgetIntegrationIntrerface ){
                    
                    KCustomWidgetIntegrationIntrerface KCustomWidget = (KCustomWidgetIntegrationIntrerface) currentComponent;

                    // check name for current PDC field being mapped
                    if( KCustomWidget.getName().equals( currentField ) ){

                        // set value
                        KCustomWidget.setValue( getFieldasString( currentField ) );

                        if( readOnlyFields.contains( currentField ) )  KCustomWidget.disable();  
                        if( editable == false ) KCustomWidget.disable();                                                                        
                        
                        break;
                    }
                                                                                   
                } // else ignore the component
                
                //----------->

                if( !radioSet && ( !components.hasNext() ) && ( missingFieldsBehavior == DONOT_IGNORE_MISSING_FIELDS ) ){ 
                    throw new KExceptionClass( 
                        " *** Can not display object *** \n " +
                        " There is no widget for [" + currentField + "]\n"  +
                        " Verify a component has the porperty 'name' assigned to ->[" + currentField + "]. \n" , null                         
                        );                    
                }
                
            } // loop component

        } // field loop
        
    }

    
//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------
    
//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------    
    
       
    public abstract void validateInput(String currentField, Component currentComponent) throws KExceptionClass;    
    
       
    public void materializeFromDisplay( 
        Container visualDisplay,        // container of destination fields
        java.util.List additionalComponentList )      // components that may not be found in the container
    throws KExceptionClass{    
        
        materializeFromDisplay( visualDisplay, additionalComponentList , DONOT_IGNORE_MISSING_FIELDS );
    }
    
    public void materializeFromDisplay( 
        Container visualDisplay,        // container of destination fields
        java.util.List additionalComponentList,       // components that may not be found in the container
        int missingFieldsBehavior  )    // Ignore fields for which there is no component?? <- for informational windows
    throws KExceptionClass
    {    
        
        // Materialize an object from visual component
        
        
// auto materialize version        
        
        //get list of components
        java.util.List widgetList = new ArrayList();
        KMetaUtilsClass.getComponentsFromContainer( visualDisplay, widgetList );                
        widgetList.addAll( additionalComponentList );
        
        // get iterator for each attribute
        Iterator fields = getFieldNamesIterator();
        while( fields.hasNext() ){  

            // for each attribute
            String currentField = ( String )fields.next(); 

            // skip versioning field
            if( currentField.equals( "object_version" ) ) continue;            
            
            // locate corresponding component
            Iterator components = widgetList.iterator();
            
            if( !components.hasNext() ) 
                throw new KExceptionClass( 
                    " *** Can not read object data *** \n " +
                    " No components in container. ", null );                   
            
            boolean radioRead = false;
            while( components.hasNext() ){ 
                                        
                // for each component
                Object currentComponent = components.next(); 
                if( currentComponent instanceof Component ) {
                    String name = ( ( Component ) currentComponent ).getName();
                    String componentValue = "";

                    // find match
                    if( name != null ){
                        if( name.equals( currentField ) ) {
                            
                            validateInput( currentField, (Component) currentComponent );                            

                            // get text from component
                            
                            // KNonVisibleBinaryWidgetClass ====================
                             if( currentComponent instanceof KNonVisibleBinaryWidgetClass ){
                                 
                                setField( currentField, ((KNonVisibleBinaryWidgetClass)currentComponent).getBinaryData() );
                                break;

                            }                            
                            
                            // JRADIO ==========================================
                            else if( currentComponent instanceof JRadioButton ){
                                 
                                radioRead = true;
                                
                                if( ((JRadioButton) currentComponent).isSelected() ){
                                    componentValue = ((JRadioButton) currentComponent).getText();
                                    setField( currentField, componentValue );      
                                    break;
                                }                                                                          
                            }
                                                                                                                   
                            // JCOMBO ==========================================
                            else if( currentComponent instanceof JComboBox )
                                componentValue = ( String )((JComboBox) currentComponent).getSelectedItem();
              
                            // JCHECK ==========================================
                            else if( currentComponent instanceof JCheckBox ){
                                
                                if( ( (JCheckBox) currentComponent ).isSelected() )                                
                                    componentValue = "Y";
                                else
                                    componentValue = "N";                                    
                                
                            // JTEXT ==========================================                                
                            }else if( currentComponent instanceof JTextComponent )
                                componentValue = ((JTextComponent)currentComponent).getText();
                           
                            // JLABEL ==========================================                                                                    
                            else if( currentComponent instanceof JLabel )
                                componentValue = ((JLabel)currentComponent).getText();
         
                            // ERROR ==========================================                                                                                                                                        
                            else
                                throw new KExceptionClass( 
                                " *** Can read data *** \n " +
                                " Invalid screen component [" + currentField + "]", null );

                            // validate field data of visual component
                            Integer fieldTypeFormat = (Integer) fieldTypes.get( currentField );

                            // if not null then there is a type specified
                            if( ( fieldTypeFormat != null ) && ( !componentValue.equals( "" ) || fieldTypeFormat.intValue()== CURRENCY_TYPE ) ){                            

                                switch( fieldTypeFormat.intValue() ){

                                    case NUMERIC_NOFORMAT : 
                                    {        
                                        componentValue = String.valueOf(
                                                (long)KMetaUtilsClass.getDecimalNumericValueFromString( componentValue, currentField ) );
                                        break;
                                    }
                                        
                                    
                                    case NUMERIC_TYPE : 
                                    case NUMERIC2_TYPE : 
                                    case NUMERIC5_TYPE :                                         
                                    {             
                                        // check value and remove format
                                        componentValue = KMetaUtilsClass.toDecimalString(                                                
                                            KMetaUtilsClass.getDecimalNumericValueFromString( 
                                                    componentValue, currentField ) );
                                        break;
                                    }

                                    case CURRENCY_TYPE : {             
                                        // check value and remove format 
                                        if ( currentField.isEmpty() )
                                            componentValue = "0";
                                        else
                                            componentValue = KMetaUtilsClass.toCurrencyString(                                                                                            
                                                KMetaUtilsClass.getCurrencyNumericValueFromString( 
                                                    componentValue, currentField ) );                                    
                                        break;
                                    }
                                    
                                    case DATE_TYPE :{  
                                        
                                        if( !componentValue.equals( "" ) ){

                                            componentValue = KMetaUtilsClass.changeDateFormat(
                                                    KMetaUtilsClass.KDEFAULT_DATE_FORMAT,
                                                    KMetaUtilsClass.KDEFAULT_LONG_DATE_TIME_FORMAT,
                                                    componentValue );                                        
                                        }                                              
                                                               
                                        break;
                                    }

                                    case TIMESTAMP_TYPE :{                                    
                                        KMetaUtilsClass.timeValidation( componentValue, currentField );                                    
                                        break;
                                    }
                                    
                                    case CAPSLETTERS_FORMAT :{                                    
                                                               
                                        break;
                                    }                                        

                                    case CAPSLETTERS_NUMBERS_FORMAT :{                                    
                                                               
                                        break;
                                    }                                        
                                               
                                        
                                    default:{
                                        throw new KExceptionClass( 
                                            " *** Cant read object *** \n " +
                                            " Invalid field type [" + currentField +
                                            "]", null
                                            );
                                    }                                                                       
                                }//end of switch
                            }//end of if

                            setField( currentField, componentValue );      

                            if( !radioRead ) break;
                        }
                    }   //end of if null
                } // if component
                else 
//                if( currentComponent instanceof dropDownFillerClass ) {
//                    
//                    if( ((dropDownFillerClass)currentComponent).getName().equals(currentField) ) {
//
//                        materializeField( currentField, 
//                            KMetaUtilsClass.toDecimalString( ((dropDownFillerClass)currentComponent).SelectedItem() ) ); 
//
//                        break;
//                    }                    
//                }else
                // KCUSTOM ==========================================                                                                                                        
                if( currentComponent instanceof KCustomWidgetIntegrationIntrerface ) {
                    
                        if( ((KCustomWidgetIntegrationIntrerface)currentComponent).getName().equals(currentField) ) {                    
                            
                            setField( currentField, 
                                ((KCustomWidgetIntegrationIntrerface)currentComponent).getValue()  ); 

                            break;
                        }
                }    
    
    //end of else
                
                if( !radioRead && ( !components.hasNext() ) && ( missingFieldsBehavior == DONOT_IGNORE_MISSING_FIELDS ) )
                   throw new KExceptionClass( 
                        " *** Can not read object *** \n " +
                        " No component for [" + currentField +"]" , null );

            } // loop

        }
        
    }
    
    //---------------------------------------------------------------------------------------------



}


