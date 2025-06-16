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

import java.io.File;
import java.util.Calendar;
import java.util.Date;
import java.util.Iterator;
import java.util.logging.Level;
import java.util.logging.Logger;


/**
	General purpose class
	System logging
*/
public class KLogClass extends persistentTextClass {

    
	// uses
	private KConfigurationClass configuration;

	// has - defaulted
	private int maxLogLines;
        private Date nextCycleTime;
        
        private String  logCycleStartTime = "";
        private int     logCyclePeriodHours = 1;
        
	// enumeration
	private int logMode;
        
        // constants
	public static final int off = 0;
	public static final int console = 1;
	public static final int file = 2;
        public static final int file_console = 3;
	public static final int dumpOnFinish = 4;

       
	//----------------------------------------------------------------------
	// INTERFACE


	/** Creates new KLogClass */
	public KLogClass( KConfigurationClass configurationParam )
	throws KExceptionClass{
		super();
		
	//uses
		configuration = configurationParam;

               
		fileName = configuration.getField( "logfile" );                
		maxLogLines = Integer.parseInt( configuration.getField( "maxloglines" ) );
                
		// log mode
		String logModeParam = configuration.getField( "log_mode" );
                
		if( logModeParam.equals( "off" ) ){

			logMode = off;
		}else
		if( logModeParam.equals( "console" ) ){

			logMode = console;
		}else
		if( logModeParam.equals( "file" ) ){

			logMode = file;
                       
		}else
		if( logModeParam.equals( "file_console" ) ){

			logMode = file_console;
		}else{		

			String message;
			message = "Invalid setting for log_mode [";
			message += logModeParam;
			message += "]";
			throw new KExceptionClass( message, null );
		};		

                //----------------------------------------------------------------------                
                
                logCycleStartTime = configuration.getField( "logcyclestarttime" );
                
                
                // next log cycle date                
                if( !logCycleStartTime.equals( "" ) ){
                    
                    logCyclePeriodHours = Integer.parseInt( configuration.getField( "logcycleperiodhours" ) );
                                    
                    // figure start time
                    String todaysDate  = KMetaUtilsClass.getCurrentDateString( "yyyy-MM-dd" );
                    nextCycleTime      = KMetaUtilsClass.stringToDate( "yyyy-MM-dd HH:mm:ss", todaysDate + " " + logCycleStartTime );                
                    
                    // find next cycle time in the future
                    java.util.Date currentTime = new Date();
                    while( nextCycleTime.before( currentTime ) ){
                        nextCycleTime = KMetaUtilsClass.addToDate( Calendar.HOUR, logCyclePeriodHours , nextCycleTime );                                                            
                    }  
                                                    
                }    
                    
	// has - defaulted                
                
                //----------------------------------------------------------------------

		add( ( Object ) "****************************************************" );
		add( ( Object ) "* LOG OPEN FOR - KFRAMEWORK 2.0                    *" );
		add( ( Object ) "****************************************************" );
                
                add( fileName + ":" + logMode );
                add( "Next Log Cycle :" + KMetaUtilsClass.dateToString( KMetaUtilsClass.KDEFAULT_LONG_DATE_TIME_FORMAT, nextCycleTime) );
                
		add( ( Object ) "" );
                
                //----------------------------------------------------------------------
                                
                
	}

	//----------------------------------------------------------------------
	/** Override add from father..
	*/
        
        @Override
	public synchronized boolean add( Object rawLogEntry ){	

                log( this, (String) rawLogEntry );
  
		return( true );
	}        
        
	//----------------------------------------------------------------------

	/** Synchronized method that adds entries to the log. */
	synchronized public void log( Object logger, String logEntry ) 
        {

                String loggerName = "Static Code";
                
                if( logger != null ) loggerName =  logger.getClass().getName();
            
		// format entry
		String formattedLogEntry = "[";
		formattedLogEntry += KMetaUtilsClass.timeStamp();
		formattedLogEntry += "][";
		formattedLogEntry += loggerName;
		formattedLogEntry += "](";
		formattedLogEntry += ( String ) logEntry;
		formattedLogEntry += ")";

		// ondebug mode
		if( logMode == off ){

                    // just save it
                    super.add( formattedLogEntry );
                    
                }else                   
		if( logMode == console ){			

                    // report 
                    System.out.println( formattedLogEntry );     
                    // save it
                    super.add( formattedLogEntry );
		}                    
		else                    
		if( logMode == file ){
		              
                    // save it
                    super.add( formattedLogEntry );
                         
		}                       
		else                                        
		if( logMode == file_console ){
                    
                    // report
                    System.out.println( formattedLogEntry );                         
                    // save it
                    super.add( formattedLogEntry );
		
		}
                
                // ================================================

                // errors always to console... rude but always good..
                if( formattedLogEntry.contains( "Exception logged! -----------------------> \n\n" ) ){
			System.out.println( formattedLogEntry );		
                }
                
                // ================================================                

                // past max size, flush ...
                if( size() > maxLogLines ) {
                    

                        if( logMode == off || logMode == console ){			

                            while( size() > maxLogLines ) remove( 0 );
                        }                    
                        else                    
                        if( logMode == file ){
                            
                            try {
                                writeToDisk();
                            } catch (KExceptionClass ex) {
                                System.out.println( "LOG ERROR!!!! =>" + ex.toString() );		
                            }                            
                            clear();   
                            super.add( "Max log lines reached, log flushed to disk..." );
                        }                       
                        else                                        
                        if( logMode == file_console ){

                            try {
                                writeToDisk();
                            } catch (KExceptionClass ex) {
                                System.out.println( "LOG ERROR!!!! =>" + ex.toString() );		
                            }
                            clear(); 
                            super.add( "Max log lines reached, log flushed to disk..." );
                        }    
                        
                }
                
                // ================================================                
                
                
	}



       //----------------------------------------------------------------------       
        
       public synchronized void addLog( KLogClass logParam)  {
           
           synchronized( logParam ){
           
               // append
               Iterator logEntry = logParam.iterator();           
               while( logEntry.hasNext() ){

                    String formattedLogEntry = ( String ) logEntry.next();
                        
                                // ondebug mode
                                if( logMode == off ){

                                    // just save it
                                    super.add( formattedLogEntry );

                                }else                   
                                if( logMode == console ){			

                                    // report 
                                    System.out.println( formattedLogEntry );     
                                    // save it
                                    super.add( formattedLogEntry );
                                }                    
                                else                    
                                if( logMode == file ){

                                    // save it
                                    super.add( formattedLogEntry );

                                }                       
                                else                                        
                                if( logMode == file_console ){

                                    // report
                                    System.out.println( formattedLogEntry );                         
                                    // save it
                                    super.add( formattedLogEntry );

                                }       
                                
                                
                                // ================================================                

                                // past max size, flush ...
                                if( size() > maxLogLines ) {


                                        if( logMode == off || logMode == console ){			

                                            while( size() > maxLogLines ) remove( 0 );
                                        }                    
                                        else                    
                                        if( logMode == file ){

                                            try {
                                                writeToDisk();
                                            } catch (KExceptionClass ex) {
                                                System.out.println( "LOG ERROR!!!! =>" + ex.toString() );		
                                            }

                                            clear();   
                                        }                       
                                        else                                        
                                        if( logMode == file_console ){

                                            try {
                                                writeToDisk();
                                            } catch (KExceptionClass ex) {
                                                System.out.println( "LOG ERROR!!!! =>" + ex.toString() );		
                                            }
                                            clear();   
                                        }                        
                                }

                                // ================================================                                   
               } 
               
             
               
               
               log( this, "<= Log appended to this log" );
           
           }// synch
                
       }        

	//----------------------------------------------------------------------

        public synchronized void dumpToConsole(){
            
            Iterator logRow = this.iterator();
            
            while( logRow.hasNext() ){
            
                System.out.println( (String) logRow.next() );
            }
        }
        
	//----------------------------------------------------------------------        
        
        // seldomly called, dont rely on this ..
	public void finalize() throws Throwable{
            
                

		add( ( Object ) "" );
		add( ( Object ) "****************************************************" );
		add( ( Object ) "* LOG CLOSED                                       *" );
		add( ( Object ) "****************************************************" );

		try{
			if(  ( logMode == file ) || ( logMode == file_console ) ) writeToDisk();

		}catch( KExceptionClass error ){};
                
                super.finalize();
	}

	//----------------------------------------------------------------------
                                    
        @Override
        public void setFileName(String fileName) {
            super.setFileName(fileName);
            log( this, "!!!Log file name changed to " + fileName );
        }
                
	//----------------------------------------------------------------------  

        public int getLogMode() {
            return logMode;
        }

        public void setLogMode(int logMode) {
            this.logMode = logMode;
        }

        public int getMaxLogLines() {
            return maxLogLines;
        }

        public void setMaxLogLines(int maxLogLines) {
            this.maxLogLines = maxLogLines;
        }

        public int getLogCyclePeriodHours() {
            return logCyclePeriodHours;
        }

        public void setLogCyclePeriodHours(int logCyclePeriodHours) {
            this.logCyclePeriodHours = logCyclePeriodHours;
        }

        public String getLogCycleStartTime() {
            return logCycleStartTime;
        }

        public void setLogCycleStartTime(String logCycleStartTime) {
            this.logCycleStartTime = logCycleStartTime;
        }
                        

	//----------------------------------------------------------------------  
        
        
        @Override
        public void writeToDisk( String fileNameParam, int log_write_modeParam ) 
        throws KExceptionClass{
                        
            synchronized( this ){
                
                super.writeToDisk( fileNameParam, log_write_modeParam );
                                                
                // check cache expired                       
                if( !logCycleStartTime.equals( "" ) ){
                    
                    Date currentTime = new Date();                    
                    if( currentTime.after( nextCycleTime ) ){
                        
                        // find next cycle time in the future
                        while( nextCycleTime.before( currentTime ) ){
                            
                            nextCycleTime = KMetaUtilsClass.addToDate( Calendar.HOUR, logCyclePeriodHours , nextCycleTime );                                                            
                        }                        
                        
                        // cycle
                        File currentLog = new File( fileNameParam );
                        File oldLog = new File( fileNameParam + ".BAK" );                        
                        // delete any previous log
                        if( oldLog.exists() ) oldLog.delete();                        
                        // save current log                        
                        currentLog.renameTo( oldLog );                        
                        
                        // on new log....
                        
                        // done                        
                        super.add( ( Object ) "" );
                        super.add( ( Object ) "****************************************************" );
                        super.add( ( Object ) "LOG CYCLED at " + KMetaUtilsClass.dateToString( KMetaUtilsClass.KDEFAULT_LONG_DATE_TIME_FORMAT, new Date() ) );
                        super.add( ( Object ) "****************************************************" );
                        super.add( fileName + ":" + logMode );
                        super.add( "=>Next log cycle :" + KMetaUtilsClass.dateToString( KMetaUtilsClass.KDEFAULT_LONG_DATE_TIME_FORMAT, nextCycleTime) );
                        
                        super.writeToDisk( fileNameParam, log_write_modeParam );
                    }                                                                                                                       
                }                
            } // synch
        }

                
	//----------------------------------------------------------------------            
        
}

