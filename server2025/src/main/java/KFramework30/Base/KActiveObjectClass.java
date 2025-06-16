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

import KFramework30.Base.KConfigurationClass;
import KFramework30.Base.KExceptionClass;
import KFramework30.Base.KLogClass;
import KFramework30.Base.KMetaUtilsClass;


import java.util.Iterator;
import java.util.Vector;
import java.util.logging.Level;
import java.util.logging.Logger;



public class KActiveObjectClass 
extends Thread {

           // thread status
           public enum statusEnum {
                              
            RESUME_REQUESTED,               
            RUNNING,
            
            PAUSE_REQUESTED,               
            PAUSED,
               
            KILL_REQUESTED,               
            KILLED                                    
           }

            // uses
            private KConfigurationClass                       configuration = null;
            private KLogClass                                 log = null;
           

            // has - defaulted
            private statusEnum status;
            private long granularity;
            private long sleepTime;
            private long nextRunTime;
            private Vector< KActiveObjectInterface >          monitors;
            
            // ========================================================================
            
            public interface KActiveObjectInterface{
                
                public void run()  throws KExceptionClass;
                public void afterPaused()  throws KExceptionClass;
            }
            
            // ========================================================================
    
            public KActiveObjectClass(
                KConfigurationClass configurationParam,
                KLogClass    logParam,               
                long period, long granularityParam
                )
                throws KExceptionClass
            {

                 // uses
                 configuration = configurationParam;
                 log = logParam;                 

                 // has
                 status = statusEnum.KILLED;
                 sleepTime = period;
                 granularity = granularityParam;                 
                 nextRunTime = KMetaUtilsClass.getCurrentTimeMilliseconds();
                 monitors = new Vector< KActiveObjectInterface >();

		 log.log( this, "Active Object Created!" );
            }
            
            //=======================================================================                        

            public void addTask( KActiveObjectInterface task ) throws KExceptionClass{
                
                if( status == statusEnum.RUNNING )
                    throw new KExceptionClass("Can not add tasks to monitor if monitor alerady running ...", null );
                        
                monitors.add( task );                
            }
            
            public void removeTask( KActiveObjectInterface task ) throws KExceptionClass{

                if( status == statusEnum.RUNNING )
                    throw new KExceptionClass("Can not add tasks to monitor if monitor alerady running ...", null );
                
                monitors.remove( task );                

            }
            
            //=======================================================================            

            
            @Override
            public synchronized void start() {
                status = statusEnum.PAUSED;                
                super.start();
            }            
            
            public synchronized void doPause(){
                status = statusEnum.PAUSE_REQUESTED;
            }            
            
            public synchronized void doResume(){
                status = statusEnum.RESUME_REQUESTED;
            }
                        
            public synchronized void doKill(){
                status = statusEnum.KILL_REQUESTED;
            }
            
            public synchronized statusEnum getStatus(){
		return( status );
            }
           
            //=======================================================================
                                
            @Override
            public void run(){
                                                
                while( true ){
                                        
                    if( KMetaUtilsClass.getCurrentTimeMilliseconds() >= nextRunTime ){
                    
                        // set next time
                        nextRunTime += sleepTime;
                        
                        if( nextRunTime < KMetaUtilsClass.getCurrentTimeMilliseconds() ){
                            nextRunTime = KMetaUtilsClass.getCurrentTimeMilliseconds() + sleepTime;
                        }

                        if( status == statusEnum.RUNNING ||  status == statusEnum.RESUME_REQUESTED  ){
                    
                            status = statusEnum.RUNNING;

                            // run
                            Iterator< KActiveObjectInterface > monitor = monitors.iterator();
                            while( monitor.hasNext() ){

                                try{

                                    monitor.next().run();                                

                                }catch( Exception error ){

                                    log.log( this, "Monitor Error -> " + KMetaUtilsClass.getStackTrace( error ) );

                                }catch( Throwable error2 ){

                                    log.log( this, "Grave Monitor Error -> " + KMetaUtilsClass.getStackTrace( error2 ) );
                                }                               

                            }
                        }// ran
                        
                    } // if timed out
                    
                    if( status == statusEnum.PAUSE_REQUESTED ){
                        status = statusEnum.PAUSED;

                        // run
                        Iterator< KActiveObjectInterface > monitor = monitors.iterator();
                        while( monitor.hasNext() ){

                            try{

                                monitor.next().afterPaused();                                

                            }catch( Exception error ){

                                log.log( this, "Monitor Error -> " + KMetaUtilsClass.getStackTrace( error ) );

                            }catch( Throwable error2 ){

                                log.log( this, "Grave Monitor Error -> " + error2  );
                            }                               

                        }


                    }

                    if( status == statusEnum.KILL_REQUESTED ) {
                        status = statusEnum.KILLED;
                        break;
                    }                    
                    
                    try{ Thread.sleep( granularity ); } catch ( InterruptedException ex ) {}
                    
                } //while true

              
                log.log( this, "Active Object Stopped...." );
            }            
            
            
            //=======================================================================            
                       
}
