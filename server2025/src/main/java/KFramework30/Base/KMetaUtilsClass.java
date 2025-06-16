
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
import javax.swing.*;
import javax.swing.event.DocumentListener;
import javax.swing.text.*; 
import java.awt.*;
import java.net.*;
import java.io.*;
import java.util.*;
import javax.swing.table.*;
import java.text.*;

// ale utils
import KFramework30.FieldValidators.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.text.SimpleDateFormat;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentEvent.EventType;
import javax.swing.text.AbstractDocument.DefaultDocumentEvent;


/**
	General purpose meta class, with some basic general methods.
*/
public class KMetaUtilsClass extends java.lang.Object {
    
    
        public static final String TRANSPORT_DIALECT_VERSION = "K5";    
    
            // dbTransaction status
        public enum parameterTypeEnum { 
            STRING,
            INT,
            LONG,
            FLOAT,
            DOUBLE,
            DATE
        };   

        // constants
	public static final int DATE_FIELD = 1;
	public static final int TIME_FIELD = 2;
        
	// uses


	// has - defaulted
	static int nextInternalFramePosition = 0;

                   
	// -------------------------------------------------------------------------

	/** Open a file from server */
	static public InputStream openRemoteFile( URL urlParam  )
	throws KExceptionClass{

		InputStream result = null;
		
		try{
			result = urlParam.openStream();

		}catch( IOException error ){

			String message = new String();
			message = "Cant open resource [";
			message += urlParam.toString();
			message += "][";
			message += error.toString();
			message += "]";
			throw new KExceptionClass( message, error );		
		};			

		return( result );
	}

        
              
	// -------------------------------------------------------------------------

	/**  Get a timestamp. Useful to stamp anything. */
	static public String timeStamp(){

		String result;

		GregorianCalendar calendar = new GregorianCalendar();

		result = String.valueOf( calendar.get( calendar.YEAR ) );
		result += "/";
		result += String.valueOf( calendar.get( calendar.MONTH ) + 1 );
		result += "/";
		result += String.valueOf( calendar.get( calendar.DAY_OF_MONTH ) );
		result += " ";
		result += String.valueOf( calendar.get( calendar.HOUR_OF_DAY ) );
		result += ":";
		result += String.valueOf( calendar.get( calendar.MINUTE ) );
		result += ":";
		result += String.valueOf( calendar.get( calendar.SECOND ) );
		result += ".";
		result += String.valueOf( calendar.get( calendar.MILLISECOND ) );
		
		return( result );
	}

	// -------------------------------------------------------------------------

	/**  Get a timestamp. Useful to stamp anything. */
	static public String time(){

		String result;

		GregorianCalendar calendar = new GregorianCalendar();

		result = String.valueOf( calendar.get( calendar.YEAR ) );
		result += "/";
		result += String.valueOf( calendar.get( calendar.MONTH ) + 1 );
		result += "/";
		result += String.valueOf( calendar.get( calendar.DAY_OF_MONTH ) );
		result += " ";
		result += String.valueOf( calendar.get( calendar.HOUR_OF_DAY ) );
		result += ":";
		result += String.valueOf( calendar.get( calendar.MINUTE ) );
		result += ":";
		result += String.valueOf( calendar.get( calendar.SECOND ) );
		
		return( result );
	}

	/**  Get a timestamp. Useful to stamp anything. */
	static public String timeOfDay(){

		String result;

                
                NumberFormat formatter = NumberFormat.getInstance();   
                formatter.setMinimumFractionDigits( 0 );
                formatter.setMaximumFractionDigits( 0 );            
                formatter.setMinimumIntegerDigits( 2 );                            
            
                                
		GregorianCalendar calendar = new GregorianCalendar();
                
                String hours = formatter.format( calendar.get( calendar.HOUR ) );
                if( hours.equals( "00" ) ) hours = "12";
                
		result = hours;
		result += ":";
		result += formatter.format( calendar.get( calendar.MINUTE ) );
		result += " ";


                
                
                if( calendar.get( calendar.AM_PM ) == calendar.AM ){
		    result += "AM";
                }else{
		    result += "PM";
                };
		
		return( result );
	}
       
        
        //---------------------------------------------------------------------------------
        
      //Final time validation
	static public String timeValidation( String timeString, String fieldName )
        throws KExceptionClass
        {      

            
            if( timeString.length() <= 5 ) {            

                String separator = timeString.substring( 2, 3 );      
                
                int separatorPosition = timeString.indexOf(":");      
                if ( !(separatorPosition == 1 || separatorPosition == 2) )
                    throw new KExceptionClass(
                        "*** Invalid Entry [" + timeString + "] on " + fieldName + "***\n" +
                        "':' Not found .\n" +
                        "[hh24:mm]"  , null                          
                    );
                    
                int hour = Integer.parseInt( timeString.substring( 0, separatorPosition ) );                
                if( hour > 24 || hour < 0 )
                    throw new KExceptionClass(
                        "*** Invalid Entry  [" + timeString + "] on " + fieldName + "***\n" +
                        "Invalid Hour.\n" +
                        "[hh24:mm]", null
                    );

                
                int minute = Integer.parseInt( timeString.substring( separatorPosition + 1, timeString.length() ) );                                 
                if( minute > 59 || minute < 0 )
                    throw new KExceptionClass(
                       "*** Invalid Entry  [" + timeString + "] on " + fieldName + "***\n" +
                        "Los minutos son invÃ¡lidos.\n" +
                        " [hh24:mm]", null
                    );
                    
            }else{
            
                throw new KExceptionClass(
                    "*** Invalid Entry  [" + timeString + "] on " + fieldName + "***\n" +
                        "Invalid Minute.\n" +
                    " [hh24:mm]", null
                );
            
            }
                        
            return( timeString );
            
        }        

	// -------------------------------------------------------------------------

	/** Returns the stack trace from any exception as a String */
	static public String getStackTrace( Throwable exceptionParam ){

		// has
		String stackTrace = new String();

		// get a buffer for print writer
		StringWriter stringBuffer = new StringWriter();
			
		// get print writer
		PrintWriter traceWriter = new PrintWriter( stringBuffer );

		// write the stack trace
		exceptionParam.printStackTrace( traceWriter );

		// save the trace
		stackTrace = stringBuffer.getBuffer().toString();

		return( stackTrace );
	}


	// -------------------------------------------------------------------------
	// GUI STUFF
	// -------------------------------------------------------------------------
	                
        static public java.awt.Window getParentWindow( java.awt.Component componentParam  ){            
            return (Frame)SwingUtilities.getAncestorOfClass(Window.class, componentParam);              
        }

        static public java.awt.Frame getParentFrame( java.awt.Component componentParam  ){            
            return (Frame)SwingUtilities.getAncestorOfClass(Frame.class, componentParam);              
        }
        
        
	// -------------------------------------------------------------------------

        static public void getComponentsFromContainer( Container container, java.util.List list)
        {
            Component[] componentArray = container.getComponents();

            for( int i=0; i < componentArray.length; i++ ) {

                //get component type
                Component currentComponent =  componentArray[ i ];          
                
                if( currentComponent.getName() != null ){
                    list.add( currentComponent );
                };                

                // re enter, 
                if( currentComponent instanceof Container )
                    getComponentsFromContainer( ( Container ) currentComponent, list );
                
            }// for

        }
        
        

	// -------------------------------------------------------------------------

	/** General message with ICON, modal type and all that.... */
	static public void showMessage( java.awt.Window parent, String message ){

			try{
                            
                                //default title and icon
                                JOptionPane.showMessageDialog( 
                                            parent,
                                            message );
			// if this also failed, theres not much you can do.
			}catch( Exception handlerError ){

				// show in console
				System.out.println( handlerError.toString() );
				System.out.println( message );

			};


	}



	// -------------------------------------------------------------------------

	/** General error message with ICON, modal type and all that.... */
	static public void showErrorMessageFromException(  java.awt.Window parent, Exception error ){
            
                        //--------------------------------------------------------            

                        String errorMessage = new String();                        

            
                        if( error instanceof KExceptionClass ){
                            
                            errorMessage = ((KExceptionClass)  error).message;

                        }else{
                            
                            errorMessage = error.toString();
                            Throwable nextError = error;
                            
                            nextError = error.getCause();
                            while( true ){                                                               
                                if( nextError == null ) break;
                                errorMessage += "\n -->" + nextError.toString();
                                nextError = nextError.getCause();
                            }
                        }
            
                        //--------------------------------------------------------
                        
			try{
                            //OKButton.setFont(new java.awt.Font ("Arial", 0, 10));
                            
                                //custom title, error icon
                                JOptionPane.showMessageDialog(
                                    parent,
                                    errorMessage,
                                    "Error",
                                    JOptionPane.ERROR_MESSAGE );
                            

			// if this also failed, theres not much you can do.
			}catch( Exception handlerError ){

				// show in console
				System.out.println( handlerError.toString() );
				System.out.println( errorMessage );

			};

                        //--------------------------------------------------------

	}

	/** General error message with ICON, modal type and all that.... */
	static public void showErrorMessageFromText1( java.awt.Window parent, String errorMessage ){
            

                        
			try{
                            //OKButton.setFont(new java.awt.Font ("Arial", 0, 10));
                            
                                //custom title, error icon
                                JOptionPane.showMessageDialog(    
                                    parent,
                                    errorMessage,
                                    "Error",
                                    JOptionPane.ERROR_MESSAGE );
                            

			// if this also failed, theres not much you can do.
			}catch( Exception handlerError ){

				// show in console
				System.out.println( handlerError.toString() );
				System.out.println( errorMessage );

			};

                        //--------------------------------------------------------

	}
        
	// -------------------------------------------------------------------------

	/** General confirmation message with ICON, modal type and all that.... */
	static public String showConfirmationMessage(  java.awt.Window parent, String message ){
           
                        String result = "";
                        
			try{
                            //Custom button text
                            Object[] options = { "OK", "CANCEL"};
                                                
                            int n = JOptionPane.showOptionDialog(
                                parent,
                                message,
                                "Confirm",
                                JOptionPane.OK_CANCEL_OPTION,
                                JOptionPane.QUESTION_MESSAGE,
                                null,
                                options, options[0] );
                            
                            if( n == 0 ) result = " OK ";
                            else result = " CANCEL ";                            


			// if this also failed, theres not much you can do.
			}catch( Exception handlerError ){

				// show in console
				System.out.println( handlerError.toString() );
				System.out.println( message );

			};


                        return( result );
	}

	// -------------------------------------------------------------------------

	/** General confirmation message with ICON, modal type and all that.... */
	static public String showGetDataMessage(  java.awt.Window parent, String title, String message ) 
                throws KExceptionClass{
           
                        String result = "";
                        
                                                
                            result = JOptionPane.showInputDialog(
                                parent,
                                message,
                                title,
                                JOptionPane.QUESTION_MESSAGE );
                            

                        return( result );
	}

             
        
	// -------------------------------------------------------------------------
	/** Generic UI open window */

	static public JInternalFrame bringInternalFrameToFront( 
		JDesktopPane desktop,
		String internalFrameTitle 
		){

                JInternalFrame result = null;
            
		// get all opened frames
		JInternalFrame[] openInternalFrames = desktop.getAllFrames();
		int totalInternalFramesOpen = 
			java.lang.reflect.Array.getLength( openInternalFrames );
		
		// look for selected window

		for( int index = 0; index < totalInternalFramesOpen; index++ ){
			if( openInternalFrames[ index ].getTitle() == internalFrameTitle ){
				// already open, bring to front
				if( openInternalFrames[ index ].isIcon() ){
					// deiconize
					try{ 
						openInternalFrames[ index ].setIcon( false );
					} catch( java.beans.PropertyVetoException denied ){ 
						/*fine*/ 
					};
                                                                                
				};
                                desktop.moveToFront( openInternalFrames[ index ] );
				openInternalFrames[ index ].show();
				
                                result = openInternalFrames[ index ];                                
			};
		};
	
		return( result );
	}

	// -------------------------------------------------------------------------

	static public void centerInScreen( Component component  ){

		//Center frame
		Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
		Dimension size = component.getSize();
		screenSize.height = screenSize.height / 2;
		screenSize.width = screenSize.width / 2;
		size.height = size.height / 2;
		size.width = size.width / 2;
		int y = screenSize.height - size.height;
		int x = screenSize.width - size.width;
		component.setLocation( x, y );
	}

	// -------------------------------------------------------------------------

            /*  KE
             *  Quite an ugly fix my dear.         
             *  I hope you can find something better.
             * This is a fix for a JTable Bug
             */
  
      static public void stopTableCellEditing( JTable visualTable )
      {
            
            int column = visualTable.getEditingColumn(); 
            if( column >= 0 ){ 

                // get custom editor if any
                TableCellEditor cellEditor = 
                    visualTable.getColumnModel().getColumn( column ).getCellEditor();

                // no custom editor, then get the default
                if( cellEditor == null ){ 
                    cellEditor = 
                        visualTable.getDefaultEditor( visualTable.getColumnClass( column ) );
                } 

                // in any case, force to shutdown, and send value to rederer.
                // This will fire editStopped(), and that will call setValue()            
                if( cellEditor != null ){ 
                        cellEditor.stopCellEditing();
                } 
            }      

      }

          
      
        //Partial time and date validation
	static public void setFieldValidator( JTextComponent textComponent, int validation )
        throws KExceptionClass
        {
            switch( validation ) {
                
                case DATE_FIELD:
                    textComponent.setDocument( 
                            new dateFieldValidatorClass( ) 
                            );                    
                    break;
                    
                case TIME_FIELD:
                    textComponent.setDocument( 
                            new timeFieldValidatorClass() 
                            );                    
                    
                    break;
                    
                default:
                    
                    throw new KExceptionClass(
                        "***Error*** Cannot apply validation to this type of field.", null
                    );                    
            } 
            
	}
        
        
	static public void setMaxCharactersFieldValidator( JTextComponent textComponent, int maxCharacters ){

		textComponent.setDocument(  new charactersFieldValidatorClass( maxCharacters ) );

	}
               
        
        //*********************        
        
	static public void setMaxNumbersFieldValidator( JTextComponent textComponent, int maxNumbers ){

		textComponent.setDocument(  new numbersFieldValidatorClass( maxNumbers, textComponent.getName() , true  ) );
                textComponent.setAlignmentY( SwingConstants.RIGHT );
	}        
        
        
	static public void setMaxNumbersNoFormatFieldValidator( JTextComponent textComponent, int maxNumbers ){

		textComponent.setDocument(  new numbersFieldValidatorClass( maxNumbers, textComponent.getName() , false ) ); 
                textComponent.setAlignmentY( SwingConstants.RIGHT );                
	} 
        

        //*********************
        
	static public void setCurrencyFieldValidator( JTextComponent textComponent, int maxNumbers ){

		textComponent.setDocument(  new currencyFieldValidatorClass1( maxNumbers, textComponent.getName() )  );
                textComponent.setAlignmentY( SwingConstants.RIGHT );
	}        

        
        public static void addOnChangeEventHandler(JTextField quantityLabel, DocumentListener labelChangeEventHandler) {

            quantityLabel.getDocument().addDocumentListener( labelChangeEventHandler );
            
            labelChangeEventHandler.changedUpdate( null );
        }
        

	static public void setCapsLettersFieldValidator( JTextComponent textComponent, int maxLetters ){

		textComponent.setDocument(  new capsLettersFieldValidatorClass( maxLetters, textComponent.getName() )  );
	}         

	static public void setCapsLettersNumbersFieldValidator( JTextComponent textComponent, int maxLetters ){

		textComponent.setDocument(  new capsLettersNumbersFieldValidatorClass( maxLetters, textComponent.getName() )  );
	}         
        
        
	// -------------------------------------------------------------------------

	static public void showInternalFrame( JDesktopPane desktop, JInternalFrame internalFrame ){

		// set size /w/h
		internalFrame.setSize( 800, 450 );	
                
                // set layer over background label
                internalFrame.setLayer( 1 );

		// add to desktop
		desktop.add( internalFrame );	

		// position in desktop
		internalFrame.setLocation( nextInternalFramePosition, nextInternalFramePosition );	
		nextInternalFramePosition += 30;
		if( nextInternalFramePosition > 200 ) nextInternalFramePosition = 0;

		// show
		internalFrame.show();
                
	}
               

	// -------------------------------------------------------------------------
        
	static public Vector parseCSV( String sourceText ){

            Vector result = new Vector();  
            
            
            String currentToken = new String();
            boolean insideQuotes = false;            
            for( int index = 0; index < sourceText.length(); index++ ){
             
                // next char
                char currentChar = sourceText.charAt( index );
                
                // no quotes parsing
                if( !insideQuotes ){  
                    
                    if( currentChar == ',' ){                        
                        
                        result.add( currentToken );
                        currentToken = "";
                        
                    }else                    
                    if( currentChar == '"' ){  
                        
                        insideQuotes = true;
                        
                    }else{
                        
                        currentToken += currentChar;
                    }
                    
                }else
                // inside quoates pasring                    
                {
                    if( currentChar == '"' ){
                        
                        insideQuotes = false;                        
                        
                    }else{
                        
                        currentToken += currentChar;                        
                    }
                }
                
            } // next char
            
            result.add( currentToken );
            
            return( result );
        }

  //-----------------------------------------------------------------
        
        static public int toJavaStaticMonth( int numberOfMonth )
        throws KExceptionClass
        {
            
            int java_static_month = -1;
            
            //get Java calendar static value for month
            switch( numberOfMonth ){            

                case 1: 
                    java_static_month = Calendar.JANUARY;
                    break;

                case 2:  
                    java_static_month = Calendar.FEBRUARY;
                    break;

                case 3:  
                    java_static_month = Calendar.MARCH;
                    break;

                case 4:  
                    java_static_month = Calendar.APRIL;
                    break;

                case 5:  
                    java_static_month = Calendar.MAY;
                    break;

                case 6:  
                    java_static_month = Calendar.JUNE;
                    break;

                case 7:  
                    java_static_month = Calendar.JULY;
                    break;        

                case 8:  
                    java_static_month = Calendar.AUGUST;
                    break;

                case 9:  
                    java_static_month = Calendar.SEPTEMBER;
                    break;

                case 10:  
                    java_static_month = Calendar.OCTOBER;
                    break;      

                case 11:  
                    java_static_month = Calendar.NOVEMBER;
                    break;

                case 12:  
                    java_static_month = Calendar.DECEMBER;
                    break;                          

                default:
                    throw new KExceptionClass( "Invalid Month [" + numberOfMonth + "]!", null );           
            }

            return java_static_month;
        }
        
        static public int toNumberOfMonth( int javaStaticMonth )
        throws KExceptionClass
        {
            
            int number_of_month = -1;
            
            //get Java calendar static value for month
            switch( javaStaticMonth ){            

                case Calendar.JANUARY: 
                    number_of_month = 1;
                    break;

                case Calendar.FEBRUARY:  
                    number_of_month = 2;
                    break;

                case Calendar.MARCH:  
                    number_of_month = 3;
                    break;

                case Calendar.APRIL:  
                    number_of_month = 4;
                    break;

                case Calendar.MAY:  
                    number_of_month = 5;
                    break;

                case Calendar.JUNE:  
                    number_of_month = 6;
                    break;

                case Calendar.JULY:  
                    number_of_month = 7;
                    break;        

                case Calendar.AUGUST:  
                    number_of_month = 8;
                    break;

                case Calendar.SEPTEMBER:  
                    number_of_month = 9;
                    break;

                case Calendar.OCTOBER:  
                    number_of_month = 10;
                    break;      

                case Calendar.NOVEMBER:  
                    number_of_month = 11;
                    break;

                case Calendar.DECEMBER:  
                    number_of_month = 12;
                    break;                          

                default:
                    throw new KExceptionClass( "Invalid Month [" + javaStaticMonth + "]!", null );           
            }
   
            
            return( number_of_month );
            
        }        


 
        // -------------------------------------------------------------------------                                        
        //Verifica si un id determinado extiste en la cadena recibida
        static public boolean isContained(long id, String idList){
            boolean resultado = false;
            String lista[] = idList.split(",");
            for (int i=0; i<lista.length; i++) {
                if ( lista[i].trim().equals( Long.toString(id) ) ) {
                    resultado = true;
                    break;
                }                
            }

            return resultado;
        }
    
        
         // -------------------------------------------------------------------------                                        
        
        public static final String[] countries = 
            {
            "United States",
            "Afghanistan",
            "Albania",
            "Algeria",
            "American Samoa",
            "Andorra",
            "Angola",
            "Anguilla",
            "Antarctica",
            "Antigua and Barbuda",
            "Argentina",
            "Armenia",
            "Aruba",
            "Australia",
            "Austria",
            "Azerbaijan",
            "Bahamas",
            "Bahrain",
            "Bangladesh",
            "Barbados",
            "Belarus",
            "Belgium",
            "Belize",
            "Benin",
            "Bermuda",
            "Bhutan",
            "Bolivia",
            "Bosnia and Herzegowina",
            "Botswana",
            "Bouvet Island",
            "Brazil",
            "British Indian Ocean Territory",
            "Brunei Darussalam",
            "Bulgaria",
            "Burkina Faso",
            "Burundi",
            "Cambodia",
            "Cameroon",
            "Canada",
            "Cape Verde",
            "Cayman Islands",
            "Central African Republic",
            "Chad",
            "Chile",
            "China",
            "Christmas Island",
            "Cocos (Keeling) Islands",
            "Colombia",
            "Comoros",
            "Congo",
            "Cook Islands",
            "Costa Rica",
            "Cote D'Ivoire",
            "Croatia (local name: Hrvatska)",
            "Cuba",
            "Cyprus",
            "Czech Republic",
            "Denmark",
            "Djibouti",
            "Dominica",
            "Dominican Republic",
            "East Timor",
            "Ecuador",
            "Egypt",
            "El Salvador",
            "Equatorial Guinea",
            "Eritrea",
            "Estonia",
            "Ethiopia",
            "Falkland Islands (Malvinas)",
            "Faroe Islands",
            "Fiji",
            "Finland",
            "France",
            "France, Metropolitan",
            "French Guiana",
            "French Polynesia",
            "French Southern Territories",
            "Gabon",
            "Gambia",
            "Georgia",
            "Germany",
            "Ghana",
            "Gibraltar",
            "Greece",
            "Greenland",
            "Grenada",
            "Guadeloupe",
            "Guam",
            "Guatemala",
            "Guinea",
            "Guinea-Bissau",
            "Guyana",
            "Haiti",
            "Heard and Mc Donald Islands",
            "Honduras",
            "Hong Kong",
            "Hungary",
            "Iceland",
            "India",
            "Indonesia",
            "Iran (Islamic Republic of)",
            "Iraq",
            "Ireland",
            "Israel",
            "Italy",
            "Jamaica",
            "Japan",
            "Jordan",
            "Kazakhstan",
            "Kenya",
            "Kiribati",
            "Korea, Democratic People's Republic of",
            "Korea, Republic of",
            "Kuwait",
            "Kyrgyzstan",
            "Lao People's Democratic Republic",
            "Latvia",
            "Lebanon",
            "Lesotho",
            "Liberia",
            "Libyan Arab Jamahiriya",
            "Liechtenstein",
            "Lithuania",
            "Luxembourg",
            "Macau",
            "Macedonia, the former Yugoslav Republic of",
            "Madagascar",
            "Malawi",
            "Malaysia",
            "Maldives",
            "Mali",
            "Malta",
            "Marshall Islands",
            "Martinique",
            "Mauritania",
            "Mauritius",
            "Mayotte",
            "Mexico",
            "Micronesia, Federated States of",
            "Moldova, Republic of",
            "Monaco",
            "Mongolia",
            "Montserrat",
            "Morocco",
            "Mozambique",
            "Myanmar",
            "Namibia",
            "Nauru",
            "Nepal",
            "Netherlands",
            "Netherlands Antilles",
            "New Caledonia",
            "New Zealand",
            "Nicaragua",
            "Niger",
            "Nigeria",
            "Niue",
            "Norfolk Island",
            "Northern Mariana Islands",
            "Norway",
            "Oman",
            "Pakistan",
            "Palau",
            "Panama",
            "Papua New Guinea",
            "Paraguay",
            "Peru",
            "Philippines",
            "Pitcairn",
            "Poland",
            "Portugal",
            "Puerto Rico",
            "Qatar",
            "Reunion",
            "Romania",
            "Russian Federation",
            "Rwanda",
            "Saint Kitts and Nevis",
            "Saint Lucia",
            "Saint Vincent and the Grenadines",
            "Samoa",
            "San Marino",
            "Sao Tome and Principe",
            "Saudi Arabia",
            "Senegal",
            "Seychelles",
            "Sierra Leone",
            "Singapore",
            "Slovakia (Slovak Republic)",
            "Slovenia",
            "Solomon Islands",
            "Somalia",
            "South Africa",
            "Spain",
            "Sri Lanka",
            "St. Helena",
            "St. Pierre and Miquelon",
            "Sudan",
            "Suriname",
            "Svalbard and Jan Mayen Islands",
            "Swaziland",
            "Sweden",
            "Switzerland",
            "Syrian Arab Republic",
            "Taiwan",
            "Tajikistan",
            "Tanzania, United Republic of",
            "Thailand",
            "Togo",
            "Tokelau",
            "Tonga",
            "Trinidad and Tobago",
            "Tunisia",
            "Turkey",
            "Turkmenistan",
            "Turks and Caicos Islands",
            "Tuvalu",
            "Uganda",
            "Ukraine",
            "United Arab Emirates",
            "United Kingdom",
            "United States Minor Outlying Islands",
            "Uruguay",
            "Uzbekistan",
            "Vanuatu",
            "Vatican City State (Holy See)",
            "Venezuela",
            "Vietnam",
            "Virgin Islands (British)",
            "Virgin Islands (U.S.)",
            "Wallis And Futuna Islands",
            "Western Sahara",
            "Yemen",
            "Yugoslavia",
            "Zaire",
            "Zambia",
            "Zimbabwe"
            };

        public static final String[] states = { 
            "N/A",
            "AL", "AK", "AZ", "AR", "AA", "AE", "AP", "CA", "CO", "CT",
            "DE", "FL", "GA", "GU", "HI", "ID", "IL", "IN", "IA", "KS",
            "KY", "LA", "ME", "MP", "MD", "MA", "MI", "MN", "MS", "MO",
            "MT", "NE", "NV", "NH", "NJ", "NM", "NY", "NC", "ND", "OH",
            "OK", "OR", "PA", "PR", "RI", "SC", "SD", "TN", "TX", "UT",
            "VI", "VT", "VA", "WA", "DC", "WV", "WI", "WY" };   
            
        // -------------------------------------------------------------------------                                        

	/**  returns the current date millisecond serial.  */
	static public long getCurrentTimeMilliseconds(){
	        		
		GregorianCalendar calendar = new GregorianCalendar();
                
		long result = calendar.getTimeInMillis();
		
		return( result );
	}        
        
        
	// -------------------------------------------------------------------------
        
        static public String byteToHexadecimal (byte character ) {
            return byteToHexadecimal( new byte[]{ character } );
        }
        

        /**
         * 
         *  Returns the hex dump from an array of bytes
         * 
         * @param noMensaje
         * @return
         */
        static public String byteToHexadecimal (byte[] noMensaje) {
            StringBuffer strMegId = new StringBuffer();
            String strHexa = "";
            if (noMensaje!=null) {
	    		for (int i=0; i<noMensaje.length; i++)
	    		{
	    			strHexa = Integer.toHexString( noMensaje[i]&255);
	    			if (strHexa.length()==1) strMegId.append("0");
	    			strMegId.append(strHexa);
	    		}
            }
    		return strMegId.toString();
        }              

        
    public static String fancyBinaryToHex(  
		byte[] source1
		){
 
		String asciipart = "";			
		String hexpart = "";					
		String result = "\n";		
		int blockCounter = 0;		
                int index = 0;
		for(  ; index < source1.length ; index++ ){	
						
			// break counter											
			blockCounter++;									
												
			//---------------------------------------------------------------						
			// get next char
			byte currentChar = source1[ index ];
			
                        // GET HEX
			hexpart += byteToHexadecimal( currentChar );
                        
			
			//GET ASCII
			if( currentChar >= '!' && currentChar <= '~' ) 
				asciipart += new String( new byte[]{ currentChar } );
			else 
				asciipart += ".";
			//---------------------------------------------------------------			
                        
			if( blockCounter == 16 ) { 
				hexpart += ":";
				asciipart += ":";
			}
                        
									
			if( blockCounter == 32 ){
				
				blockCounter = 0;				
				
				// block indexer																
				result += ( index + "      " ).substring( 0 ,4 );
                                
				result += "[";
				result += hexpart;
				result += "] ";
				result += "[";
				result += asciipart;
				result += "]\n";
				
				hexpart = "";
				asciipart = "";
				
			} 
				
			
						
		};	// for
			
		if( hexpart != "" ){			
			result += ( index + "      " ).substring( 0 ,4 );
                        
                        result += "[";
			result += hexpart;
			result += "] ";
			result += "[";
			result += asciipart;
			result += "]\n";
		}
	
	
		return( result );	
	};            
        
        /**
         * 
         *  Handy  func that also checks for the string "NULL"
         * 
         */        
    static public boolean isEmptyString(String value) {

        boolean bReturn = true;

        if (value != null) {
            if (!value.trim().equalsIgnoreCase("NULL")) {
                if (((value.trim()).length() >= 1)) {
                    bReturn = false;
                }
            }
        }

        return bReturn;
    }

    //--------------------------------------------------------------------------------
    // DATES
    
    
	static public final String DISPLAY_HOUR_FORMAT = "HH:MM";                 
	static public final String KDEFAULT_LONG_DATE_TIME_FORMAT = "yyyy-MM-dd HH:mm:ss";                                                
        static public final String KDEFAULT_DATE_FORMAT = "yyyy-MM-dd";                                                        
        static public final long   MILLSECS_PER_DAY = 1000 * 60 * 60 * 24; 

        //--
        static public java.util.Date addToDate( int timeUnit, int value, Date date ){
            
            Calendar calendar = Calendar.getInstance();                    
            calendar.setTime( date ); 
            calendar.add( timeUnit, value );                        
            return( calendar.getTime() );
        };
        
        // -- Cambia el formato de fecha 
        static public String changeDateFormat( String sourceFormat, String targetFormat, String value ) 
                throws KExceptionClass{
            String resultado = "";
            SimpleDateFormat fmtOrigen = new SimpleDateFormat(sourceFormat);
            SimpleDateFormat fmtDestino = new SimpleDateFormat(targetFormat);
            try {
                resultado = fmtDestino.format( fmtOrigen.parse(value) );
            }catch (Exception e) {
               throw new KExceptionClass( " Invalid Date ", e );            
            }
            return resultado;
        }

        // -- Formatea una fecha  
         static public String dateToString( String targetFormat, java.util.Date value ) {
            if( value == null ) return( new String() );
            SimpleDateFormat fmtDestino = new SimpleDateFormat(targetFormat);
            return fmtDestino.format( value );
        }

         // -- Conviete la cadena a fecha de acuerdo al formato 
         static public java.util.Date stringToDate( String sourceFormat, String  value ) throws KExceptionClass {
             
            java.util.Date resultado = null;
            
            if (value!=null) {
                try {
                    
                    SimpleDateFormat fmtOrigen = new SimpleDateFormat(sourceFormat);
                    resultado = fmtOrigen.parse(value) ;
                    
                }catch (Exception e) {
                   throw new KExceptionClass( " Invalid Date ", e );            
                }
            }
            
            return resultado;
        }
         
         /** Get currrent date */
         static public java.util.Date getCurrentDate( ) {
             
                return new java.util.Date();
         }
         
         
         /** Get currrent date-- */
         static public String getCurrentDateString(String formatoFecha) {
                java.util.Date fecha = new java.util.Date();
                SimpleDateFormat formateador = new SimpleDateFormat (formatoFecha);
                String fechaFormateada = formateador.format(fecha);
                return fechaFormateada;
         }         



    
    //--------------------------------------------------------------------------------
    // NUMBERS
         
        // =============================================================================

        static public int getSmallIntegralNumericValueFromString( String numberString )
        throws KExceptionClass        
        {        
            
            return( (int) getIntegralNumericValueFromString( numberString, "" ) );
        }
         
         
        static public long getIntegralNumericValueFromString( String numberString )
        throws KExceptionClass        
        {        
            
            return( getIntegralNumericValueFromString( numberString, "" ) );
        }
        
        static public long getIntegralNumericValueFromString( String numberString, String fieldName )
        throws KExceptionClass
        {
            long result;
            
            if( numberString.equals( "" ) ) return 0;
            
            try{
                
                Locale currentLocale = Locale.getDefault();               
                Number resultNumber = NumberFormat.getNumberInstance( currentLocale ).parse( numberString );              
                result = resultNumber.longValue();                                               
                                
            }catch( Exception error ){
                

                // its not a currency either, well then we dont know what is it
                throw new KExceptionClass(
                    "\n*** Invalid Number. ***\n" +
                    "[" + fieldName + "]", error
                );
                                              
            }
            
            return( result );
        }        
        
        
        
        // =============================================================================        
        
        static public double getDecimalNumericValueFromString( String numberString )
        throws KExceptionClass        
        {        
            
            return( getDecimalNumericValueFromString( numberString, "" ) );
        }
        
        static public double getDecimalNumericValueFromString( String numberString, String fieldName )
        throws KExceptionClass
        {
           double result;
           
           if( numberString.equals( "" ) ) return 0;           
           
           try{
                
               try{
                                  
                // first try regular number
                Locale currentLocale = Locale.getDefault();               
                Number resultNumber = NumberFormat.getNumberInstance( currentLocale ).parse( numberString );              
                result = resultNumber.doubleValue();                                               
                
               }catch( Exception numberRead ){
                   
                    // mhh, well maybe its a currency number...
                    Locale currentLocale = Locale.getDefault();               
                    Number resultNumber = NumberFormat.getCurrencyInstance( currentLocale ).parse( numberString ); 
                    result = resultNumber.doubleValue();                   
                   
               }
                
            }catch( Exception error ){
                

                // its not a currency either, well then we dont know what is it
                throw new KExceptionClass(
                    "\n*** Invalid Number. ***\n" +
                    "[" + fieldName + "]", error
                );
                    

            }
            
            return( result );

        }
        
        
        //= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
                
        static public double getCurrencyNumericValueFromString( String numberString )
        throws KExceptionClass        
        {        
            
            return( getDecimalNumericValueFromString( numberString, "" ) );
        }
        
        static public double getCurrencyNumericValueFromString( String numberString, String fieldName )
        throws KExceptionClass
        {
           double result;
           
           if( numberString.equals( "" ) ) return 0;           
           
           try{
               
               try{
                   
                    // get current locale
                    Locale currentLocale = Locale.getDefault();               
                    Number resultNumber = NumberFormat.getCurrencyInstance( currentLocale ).parse( numberString ); 
                    result = resultNumber.doubleValue();                   
                   
               }catch( Exception numberRead ){
                   
                    // not a currency maybe a plain number
                    Locale currentLocale = Locale.getDefault();               
                    Number resultNumber = NumberFormat.getNumberInstance( currentLocale ).parse( numberString );              
                    result = resultNumber.doubleValue();                                               
                   
               }
                                    

            }catch( Exception error2 ){

                // its not a currency either, well then we dont know what is it
                throw new KExceptionClass(
                    "\n*** Invalid Number. ***\n" +
                    "[" + fieldName + "]", error2
                );                                    
            }
                       
            return( result );
        }
        
        // = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
        
        static public String toCurrencyString( String value ) throws KExceptionClass{    
                        
            return( toCurrencyString( getDecimalNumericValueFromString( value ) ) );
        }    
        
        static public String toCurrencyString( double value ){
            
            NumberFormat formatter = NumberFormat.getCurrencyInstance( Locale.getDefault() );
                       
            return( formatter.format( value ) );
            
        }
        
        static public String toDecimalString( double value, int fractions ){
            
            NumberFormat formatter = NumberFormat.getInstance( Locale.getDefault() );
            
            formatter.setMinimumFractionDigits( fractions );
            formatter.setMaximumFractionDigits( fractions );            
            
            return( formatter.format( value ) );
            
        }
        
        static public String toDecimalString( double value ){
            
            NumberFormat formatter = NumberFormat.getInstance( Locale.getDefault() );
                        
            return( formatter.format( value ) );
            
        }        

        // NUMBERS        
        //---------------------------------------------------------------------------        

        
               
        private static JProgressBar mainProgressBar;
        
        public static JProgressBar getMainProgressBar() {
            return mainProgressBar;
        }

        public static void setMainProgressBar(JProgressBar mainProgressBar) {
            KMetaUtilsClass.mainProgressBar = mainProgressBar;
            mainProgressBar.setMaximum( 100 );
            mainProgressBar.setMinimum( 0 );
            mainProgressBar.setValue( 0 );
        }        
        
        public static void setProgressBarValue1( int progressIndex ){
            if( mainProgressBar != null){
                mainProgressBar.setValue( progressIndex );
                mainProgressBar.paint( mainProgressBar.getGraphics() );                    
            }
        }
        
        //---------------------------------------------------------------------------                        
        // PROGRESS ANIMATION  
        
        private static  JLabel statusAnimationLabel;
        private static  KActiveObjectClass busyIconTimer;
        private static  Icon idleIcon;
        private static  Icon[] busyIcons = new Icon[15];
        private static  int busyIconIndex = 0;

        
                // inner busy icon animation thread....
                public static class statusAnimationThread implements KActiveObjectClass.KActiveObjectInterface {
                    public void run() throws KExceptionClass {
                            busyIconIndex = (busyIconIndex + 1) % busyIcons.length;
                            statusAnimationLabel.setIcon(busyIcons[busyIconIndex]);
                            statusAnimationLabel.paint(statusAnimationLabel.getGraphics()); 
                    }

                    public void afterPaused() throws KExceptionClass {
                            statusAnimationLabel.setIcon( idleIcon );
                            statusAnimationLabel.paint(statusAnimationLabel.getGraphics());
                    }
                }
        
        public static void setMainProgressIcon( 
                Component component,
                JLabel statusAnimationLabelParam, KConfigurationClass configuration, KLogClass log ) 
                throws KExceptionClass{
            
            statusAnimationLabel = statusAnimationLabelParam;
                               
            // load icons
            for (int i = 0; i < busyIcons.length; i++) {                
                busyIcons[i] = new javax.swing.ImageIcon( component.getClass().getResource("/resources/busyicons/busy-icon" + i +".png"));
            }
            
            // build thread                        
            busyIconTimer = new KActiveObjectClass( configuration, log, 30, 10 );                        
            busyIconTimer.addTask( new statusAnimationThread() );
            busyIconTimer.start();
            
            // initialize to idel
            idleIcon = new javax.swing.ImageIcon( component.getClass().getResource("/resources/busyicons/idle-icon.png"));            
            statusAnimationLabel.setIcon( idleIcon );
        }

        //------------------------------------------------------------------
        
        // control
        
        public static void cursorWait( Window component ){            
            component.setCursor( new Cursor( Cursor.WAIT_CURSOR )  );
            if( statusAnimationLabel != null  ){
                busyIconTimer.doResume();
            }
        }                
        
        public static void cursorNormal( Window component ){            
            component.setCursor( new Cursor( Cursor.DEFAULT_CURSOR )  );  
            
            if( statusAnimationLabel != null  ){
                busyIconTimer.doPause();
            }
            
        }
        
        //---------------------------------------------------------------------------                
        
}




