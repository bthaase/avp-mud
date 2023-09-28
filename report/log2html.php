<?php
 class AvP_Match
 {
     // Flags
     var $bots = "No";
     
     // Arena Data
     var $arena = "";
     
     var $date = "";
     
     var $winner = 0;
     
     var $player_cap = 0;
     
     var $time_limit = 0;
     
     var $total_kills = array( 0, 0, 0 );
     
     var $items = array( );
     var $kills = array( );
     var $deaths = array( );
     var $players = array( );
     
     var $complete = FALSE;
 
     function add_player( $Name )
     {
        $Field = explode( ":", $Item );
     
        if ( $Field[0] == "NPC" ) return;
	
	for( $i = 0; $i < count($this->players); $i++ )
        {
            $Submatch = explode( ":", $Match->items[$i] );
               if ( $Submatch[1] == $CItem[1] )
               {
                  $Match->items[$i] = implode(":", array( $Submatch[0], $Submatch[1], $Submatch[2] + 1 ));
                  $Item = "";
                  break;
               }
             }   
                            
             if ( $Item <> "" ) array_push( $Match->items, $Item );
                  
     }
 }
 
 function slice( $Str )
 {   
     $PntA = 0;
     $PntB = 0;
     $Quote = 0;
     $Done = FALSE;
     
     $i = 0;
     
     while ( !$Done )
     {
        $PntA = (int)(strpos( $Str, " ", $i ));
        $PntB = (int)(strpos( $Str, "'", $i ));
        
        if ( $PntA == $PntB && $PntA == 0 )
        {
           $Done = TRUE;
        }
        else if ( ( $PntA < $PntB && $PntA > 0 ) || ( $PntA > 0 && $PntB <= 0 ) )
        {
           if ( $Quote == 0 ) $Str = substr_replace( $Str, "~", $PntA, 1 ); 
           $i = $PntA + 1;
        }
        else if ( ( $PntB < $PntA && $PntB > 0 ) || ( $PntB > 0 && $PntA <= 0 ) )
        {
           $Quote = 1 - $Quote;
           $i = $PntB + 1;
        }
        else
        {
           $Done = TRUE;
        }
     }
         
     return explode( "~", $Str );
 }
 
 function assimilate_stream( $Log )
 {
     $Match = new AvP_Match;
   
     for ( $Line = 0; $Line < count($Log); $Line++ )
     {
        $Fields = explode( ";", $Log[$Line] );
        $Blocks = slice( $Fields[2] );
        
        // Process the Log
        switch( $Fields[1] )
        {
          case "CONTROL":    // Control Events    
             if (trim($Blocks[0]) == "Currently")
             {
                if ( $Blocks[1] > $Match->player_cap ) $Match->player_cap = $Blocks[1];
             }
             break;
          case "EVENT":      // Generic Events
             break;
          case "KILL":       // Kill Events
             $PC = str_replace( "'", "", $Blocks[0] ) . ":1";
	     $Vic = str_replace( "'", "", $Blocks[2] ) . ":1";
             
	     $CItem = explode( ":", $Item );
     
             for( $i = 0; $i < count($Match->items); $i++ )
             {
               $Submatch = explode( ":", $Match->items[$i] );
               if ( $Submatch[1] == $CItem[1] )
               {
                  $Match->items[$i] = implode(":", array( $Submatch[0], $Submatch[1], $Submatch[2] + 1 ));
                  $Item = "";
                  break;
               }
             }   
                            
             if ( $Item <> "" ) array_push( $Match->items, $Item );
             
	     break;
          case "DEATH":      // Death Events
             break;
          case "ITEM":       // Item Use Events
             $Item = str_replace( "'", "", $Blocks[2] ) . ":1";
             $CItem = explode( ":", $Item );
     
             for( $i = 0; $i < count($Match->items); $i++ )
             {
               $Submatch = explode( ":", $Match->items[$i] );
               if ( $Submatch[1] == $CItem[1] )
               {
                  $Match->items[$i] = implode(":", array( $Submatch[0], $Submatch[1], $Submatch[2] + 1 ));
                  $Item = "";
                  break;
               }
             }   
                            
             if ( $Item <> "" ) array_push( $Match->items, $Item );
             
             break;
          case "GRENADE":    // Grenade Blast Events
             break;
          case "ARENATIME":
             $Match->time_limit = $Blocks[3];
             break;
          case "ARENANAME":
             $Match->arena = trim(substr($Fields[2], strlen("Current Arena Is ")));
             break;
        }
     }
     
     return $Match;
 }
 
 function generate_report( $ID )
 {
     $Races = array( "Marine", "Alien", "Predator" );
     
     $In = file( $ID . ".log" );
     $Out = fopen( $ID . ".html", "w" );
    
     // Process the log file
     $Match = assimilate_stream( $In );

     //
     // Start writing the HTML Report
     //
     fwrite( $Out, "<HTML><HEAD><TITLE>\n" );
     
     // Output the Report Title
     fwrite( $Out, "AvP Report Engine [Alpha] | Match: " . $Match->arena );

     fwrite( $Out, '</TITLE><LINK REL="stylesheet" HREF="style.css">');
     fwrite( $Out, "</HEAD><BODY><DIV ALIGN='CENTER'>\n" );

     // Report Header
     // -------------------------------------------------------------
     fwrite( $Out, "<TABLE WIDTH='500' CELLSPACING='0' CELLPADDING='0' CLASS='HEADER'><TR><TD>\n" );
     fwrite( $Out, "AvPStats -\n" );
     fwrite( $Out, "</TD></TR></TABLE>\n" );
     
     // Game Summary
     // -------------------------------------------------------------     
     fwrite( $Out, "<TABLE WIDTH='500' CLASS='SUMMARY'><TR><TD><CENTER>\n" );
     fwrite( $Out, "<SPAN CLASS='TITLE'>$Match->arena</SPAN><BR>Game ID - $ID<BR><SPAN CLASS='WINNER'>\n" );
     fwrite( $Out, $Races[$Match->winner] . "s are victorious with " . $Match->total_kills[$Match->winner] . " kills.</SPAN><BR>\n" );
     fwrite( $Out, "</CENTER></TD></TR></TABLE>\n" );

     // Game Information
     // -------------------------------------------------------------     
     fwrite( $Out, "<TABLE WIDTH='500' CELLSPACING='0' CELLPADDING='0' CLASS='CAPTION'><TR><TD>" );
     fwrite( $Out, "Game Information</TD></TR></TABLE>");

     fwrite( $Out, "<TABLE WIDTH='500' CLASS='INFORMATION'><TR><TD>\n" );
     fwrite( $Out, "<B>Match Date: </B>" . $Match->date . "<BR>\n" );
     fwrite( $Out, "<B>Arena Name: </B>" . $Match->arena . "<BR>\n" );
     fwrite( $Out, "<B>Maximum Players: </B>" . $Match->player_cap . "<BR>\n" );
     fwrite( $Out, "<B>Time Limit: </B>" . $Match->time_limit . "<BR>\n" );
     fwrite( $Out, "<B>Bots: </B>" . $Match->bots . "<BR>\n" );
     fwrite( $Out, "</TD></TR></TABLE>\n" );
     
     // Player Summary
     // -------------------------------------------------------------     
     fwrite( $Out, "<TABLE WIDTH='500' CELLSPACING='0' CELLPADDING='0' CLASS='CAPTION'><TR><TD>" );
     fwrite( $Out, "Player Information</TD></TR></TABLE>");

     fwrite( $Out, "<TABLE WIDTH='500' CLASS='PLAYERS'><TR><TD>\n" );
     fwrite( $Out, "Information Pending.<BR>\n" );
     fwrite( $Out, "</TD></TR></TABLE>\n" );

     // Special Events
     // -------------------------------------------------------------     
     fwrite( $Out, "<TABLE WIDTH='500' CELLSPACING='0' CELLPADDING='0' CLASS='CAPTION'><TR><TD>" );
     fwrite( $Out, "Special Events And Awards</TD></TR></TABLE>");

     fwrite( $Out, "<TABLE WIDTH='500' CLASS='SPECIAL'><TR>\n" );
     fwrite( $Out, "<TD ALIGN='LEFT'><B>First Blood:</B></TD><TD ALIGN='CENTER'>" . "..." . "</TD><TD>&nbsp;&nbsp;</TD>\n" );
     fwrite( $Out, "<TD ALIGN='LEFT'><B>Double Kills:</B></TD><TD ALIGN='CENTER'>" . "0" . "</TD><TD>&nbsp;&nbsp;</TD>\n" );
     fwrite( $Out, "<TD ALIGN='LEFT'><B>Multi Kills:</B></TD><TD ALIGN='CENTER'>" . "0" . "</TD><TD>&nbsp;&nbsp;</TD>\n" );
     fwrite( $Out, "<TD>&nbsp;&nbsp;</TD></TR><TR>" );
     fwrite( $Out, "<TD ALIGN='LEFT'><B>Ultra Kills:</B></TD><TD ALIGN='CENTER'>" . "0" . "</TD><TD>&nbsp;&nbsp;</TD>\n" );
     fwrite( $Out, "<TD ALIGN='LEFT'><B>Monster Kills:</B></TD><TD ALIGN='CENTER'>" . "0" . "</TD><TD>&nbsp;&nbsp;</TD>\n" );
     fwrite( $Out, "<TD ALIGN='LEFT'><B>&nbsp;</B></TD><TD ALIGN='CENTER'>" . "&nbsp;" . "</TD><TD>&nbsp;&nbsp;</TD>\n" );
     fwrite( $Out, "<TD>&nbsp;&nbsp;</TD></TR></TABLE>\n" );

     // Murder Matchup
     // -------------------------------------------------------------     
     fwrite( $Out, "<TABLE WIDTH='500' CELLSPACING='0' CELLPADDING='0' CLASS='CAPTION'><TR><TD>" );
     fwrite( $Out, "Murder Matchup</TD></TR></TABLE>");

     fwrite( $Out, "<TABLE WIDTH='500' CLASS='MURDER'><TR><TD>\n" );
     fwrite( $Out, "Information Pending.<BR>\n" );
     fwrite( $Out, "</TD></TR></TABLE>\n" );

     // Weapon Specifics
     // -------------------------------------------------------------     
     fwrite( $Out, "<TABLE WIDTH='500' CELLSPACING='0' CELLPADDING='0' CLASS='CAPTION'><TR><TD>" );
     fwrite( $Out, "Weapon Specifics</TD></TR></TABLE>");

     fwrite( $Out, "<TABLE WIDTH='500' CLASS='WEAPONS'><TR><TD>\n" );
     fwrite( $Out, "Information Pending.<BR>\n" );
     fwrite( $Out, "</TD></TR></TABLE>\n" );

     // Item Usage
     // -------------------------------------------------------------     
     fwrite( $Out, "<TABLE WIDTH='500' CELLSPACING='0' CELLPADDING='0' CLASS='CAPTION'><TR><TD>" );
     fwrite( $Out, "Item Usage</TD></TR></TABLE>");

     fwrite( $Out, "<TABLE WIDTH='500' CLASS='ITEMS'>\n" );
     fwrite( $Out, "<TR><TD COLSPAN='3'><B>Items Used:</B></TD></TR>" );
     for ( $i = 0; $i < count($Match->items); $i++ )
     {
        $Item = explode( ":", $Match->items[$i] );
        
        fwrite( $Out, "<TR><TD WIDTH='16'><IMG WIDTH='16' HEIGHT='16' SRC='images/" . $Item[0] . ".gif'></TD>\n" );
        fwrite( $Out, "<TD>" . $Item[1] . "</TD>" );
        fwrite( $Out, "<TD ALIGN='RIGHT'>" . $Item[2] . "</TD></TR>" );
     }
          
     if ( $i <= 0 ) fwrite( $Out, "<TR><TD WIDTH='16'><IMG WIDTH='16' HEIGHT='16' SRC='images/blank.gif'></TD><TD>No items were used during this match.</TD><TD>-</TD></TR>\n" );   
     fwrite( $Out, "</TABLE>\n" );

     // Report Footer
     // -------------------------------------------------------------
     fwrite( $Out, "<TABLE WIDTH='500' CLASS='FOOTER'><TR><TD>\n" );
     fwrite( $Out, "<CENTER>Report Generated by AvPStats and the AvP Game Engine.</CENTER>\n" );
     fwrite( $Out, "</TD></TR></TABLE>\n" );

     fwrite( $Out, "</DIV></BODY></HTML>\n" );

     // fclose( $In );
     fclose( $Out );
 }

 generate_report( "08142004105005" );
?>
