import processing.serial.*;

final boolean DEBUG = false;
final boolean GRANX = true;
final boolean LOGGING = true;
final boolean SHIFT_GRANX = true;


boolean legend_group_flip = false;
long legend_group_flip_timelast = millis();
 
float STRESS_SCALING = 1.0;
final int IMAGE_DELAY = 200;

final int MAX_PERFORMERS = 6;

Serial myPort;    // The serial port
String inString;  // Input string from serial port
int lf = 10;      // ASCII linefeed

final int IMAGE_X = 800; // 180;
final int IMAGE_Y = 0;
final int IMAGE_WIDTH = 800; //600;
final int IMAGE_HEIGHT = 800; //600;
final int GRAPHS_WIDTH = 780; //170;
final int GRAPHS_HEIGHT = 650; //500;
final int IND_GRAPH_HEIGHT = 100; //80;

final String performer_names[] = {"Soprano","Mezzo Soprano","Counter Tenor","Bass","Choir 1","Choir 2"};

Table Logtable;


/**
 * Load and Display 
 * 
 * Images can be loaded and displayed to the screen at their actual size
 * or any other size. 
 */
  byte           nodeId; //store this nodeId
  byte           packetId;
  int          bpm;
  int          mov;
  float             batt;
  float             temp;
  int          gsr;

  int[]          mapped_bpm = new int[MAX_PERFORMERS];
  int[]          mapped_mov = new int[MAX_PERFORMERS];
  int[]          mapped_temp = new int[MAX_PERFORMERS];
  int[]          mapped_gsr = new int[MAX_PERFORMERS];

  int[]          raw_packetid = new int[MAX_PERFORMERS];
  int[]          raw_bpm = new int[MAX_PERFORMERS];
  int[]          raw_mov = new int[MAX_PERFORMERS];
  int[]          raw_bat = new int[MAX_PERFORMERS];
  int[]          raw_temp = new int[MAX_PERFORMERS];
  int[]          raw_gsr = new int[MAX_PERFORMERS];

  int[]          anx_levels = new int[MAX_PERFORMERS];   // one for each performer

  int image_chosen = 0;
  int level_chosen = 1;

  float f_total_anx;
  int total_anx;
  int noof_performers;
  
  color red = color(255, 0, 0);
  color green = color(0, 255, 0);
  color blue = color(0, 0, 255);
  color purple = color(255, 0, 255);

  boolean debug = true;

  int dat[]; // array of numbers read in on one line from serial port


  // holds the current directory chosen
  // this cycles through all directories (when an image is loaded)
  //int directory_chosen = 0;  //0 = doc  1 = med  2 = stat  3 = num

  int imgflag = 0;
  PImage theimg;
  int dir_noof_images[] = {6,5,7,7,11,9,8,6,5,2};
  
  long timelast = millis();
  
  int graphplot_x = 0;
  
  
void setup() {
 fullScreen(SPAN);
//  size(1000, 800);

  // List all the available serial ports:
  printArray(Serial.list());
  println();
  // Open the port you are using at the rate you want:
  myPort = new Serial(this, Serial.list()[1], 115200);
  myPort.bufferUntil(lf);
  
  background(0);
  stroke(255);

  // reset anx levels for performers ( 0 = invalid performer/no data received)
   for(int f = 0; f < MAX_PERFORMERS; f++)
   {
      anx_levels[f] = 0;
      mapped_bpm[f] = 0;
      mapped_mov[f] = 0;
      mapped_temp[f] = 0;
      mapped_gsr[f] = 0;
      raw_packetid[f] = 0;
      raw_bpm[f] = 0;
      raw_mov[f] = 0;
      raw_bat[f] = 0;
      raw_temp[f] = 0;
      raw_gsr[f] = 0;
   } 
   
   
   Logtable = new Table();
  
  Logtable.addColumn("id");
  Logtable.addColumn("packetid");
  Logtable.addColumn("bpm");
  Logtable.addColumn("mov");
  Logtable.addColumn("bat");
  Logtable.addColumn("temp");
  Logtable.addColumn("gsr");
  
  // Start with blank screen
  current_alpha = 0;
   
}


int current_alpha = 255;
int total_anx_last = 0;

boolean started = false;

void draw() 
{
  if(started)
  {
    
      if (inString != null)
      { 
        if(debug)print("SERIAL:" + inString);  // show the line of serial input
        int chars = inString.length();
        String inB2 = inString.substring(0, chars-2); // modified input line, without CR/LF
        dat = int(split(inB2, ','));  // parse comma-separated number string into numbers
        if(debug)println("DAT_IN:" + dat[0] + "," + dat[1] + "," + dat[2] + "," + dat[3] + "," + dat[4] + "," + dat[5] + "," + dat[6] );
        if(debug)println(); 
       
        inString  = null;
    
        
    
        int theID =dat[0] - 90; 
    
        if(theID >= 0 && theID < MAX_PERFORMERS)
        {    
            // Copy sensor data into relevant ID array
            read_sensor(theID);
           
            if(LOGGING)
              log_sensor(theID);
            
            // Work out anxiety level for the current performer ID
            // map sensor readings to 1 - 10 anxiety levels
            map_sensors(theID);
           
            // Store an anxiety level for the performer ID
            anx_levels[theID] = (mapped_bpm[theID] + mapped_mov[theID] + mapped_temp[theID] + mapped_gsr[theID]) / 4; 
      //      anx_levels[theID] = (mapped_bpm[theID] + mapped_mov[theID]*2 ) / 2; 
        }
        
        // Use non-zero performer anx levels to get an average anx level
        // for the whole group of performers
        total_anx = 0;
        noof_performers = 0;
        for(int f = 0; f < MAX_PERFORMERS; f++)
        {
          if(anx_levels[f] > 0)
          {
            if(DEBUG)
            {
              print("Performer:" + f + " AX:" + anx_levels[f] + " ");
              print(" BT:" + raw_bat[f] + "   ");
            }        
            total_anx += anx_levels[f];
            noof_performers++;
          }
        }
       
        // Now get average anx for all valid performers
        total_anx = total_anx / noof_performers;

        f_total_anx =  total_anx / noof_performers;
        if(f_total_anx > 9.5)
            total_anx = 10;
        
        //scale to get 10 from stressed performers
        total_anx  *=STRESS_SCALING;  
    
        if(total_anx > 10)
          total_anx = 10;
        
        if(DEBUG)
        {
          println("   Performers:" + noof_performers + "  GROUP ANXIETY:" + total_anx);
          println();
        }
    
        // PLOT the raw data graphs
        for(int gid = 0; gid < MAX_PERFORMERS; gid++)
        {
          if(anx_levels[gid] > 0) plot_raw_data(gid);
        }
        
        graphplot_x++;
        if(graphplot_x == GRAPHS_WIDTH)
        {
             // clear display
             fill(0);
             stroke(0);
             rectMode(CORNER);
             rect(0, 0, GRAPHS_WIDTH, GRAPHS_HEIGHT);
             graphplot_x = 0;
             draw_graph_bounds();
      }
    
        // Show debug info   
        if(DEBUG) draw_debug(500,GRAPHS_HEIGHT);
        if(GRANX)
        {
          if(SHIFT_GRANX || millis() - legend_group_flip_timelast > 12000)
          {
           legend_group_flip_timelast = millis();
           legend_group_flip = !legend_group_flip;
          }
           if(legend_group_flip == false)
           {
            if(SHIFT_GRANX)
             draw_groupanx(240,GRAPHS_HEIGHT);
            else
             draw_groupanx(00,GRAPHS_HEIGHT);
           }
           else
           {
             draw_legend(0, GRAPHS_HEIGHT);
           }
        }
      }
    
      
        // IF group anxiety has changed then start loading a new image
     //   if(total_anx != total_anx_last && current_alpha>=255 && millis() - timelast > 5000)
        if(current_alpha>=255 && millis() - timelast > 10000)
        {
           timelast = millis();
           total_anx_last = total_anx;
//           load_new_group_image( total_anx);
           current_alpha = 0;
        }
      
      
      // Update group image fading
      if(current_alpha < 255)
      {
          tint(255, current_alpha);
 //         image(theimg, IMAGE_X, IMAGE_Y, IMAGE_WIDTH, IMAGE_HEIGHT);  
          current_alpha = current_alpha+10;
      }
  }
  else
  {
      if(current_alpha < 255)
      {
          fill(current_alpha);
          stroke(current_alpha);
          textSize(48);
          text("Anxiety Fanfare",300,300);
          current_alpha = current_alpha+10;
          delay(100);
      }
  }
}

void log_sensor(int theID)
{
  TableRow newRow = Logtable.addRow();
  newRow.setInt("id", theID);
  newRow.setInt("packetid", raw_packetid[theID]);
  newRow.setInt("bpm", raw_bpm[theID]);
  newRow.setInt("mov", raw_mov[theID]);
  newRow.setInt("bat", raw_bat[theID]);
  newRow.setInt("temp", raw_temp[theID]);
  newRow.setInt("gsr", raw_gsr[theID]);

  
}

boolean tablesaved = false;

void keyPressed() 
{
  if(key == 's' && tablesaved == false)
  {
      tablesaved = true;
      saveTable(Logtable, "data/biodata.csv");   
  }
  else if(key == 'p')
  {
    STRESS_SCALING += 0.1;
    if(STRESS_SCALING > 2.0)
      STRESS_SCALING = 2.0;
  }
  else if(key == 'l')
  {
    STRESS_SCALING -= 0.1;
    if(STRESS_SCALING < 1.0)
      STRESS_SCALING = 1.0;
  }
  else if(key == 'g' && started == false)
  {
     background(0);
     draw_legend(0, GRAPHS_HEIGHT);
     draw_graph_bounds();
     started = true;
  }
}


void draw_debug(int x, int y)
{
     fill(0);
     stroke(50);
     rectMode(CORNER);
     rect( x, y, 128, 128);
     textSize(32);
     fill(255);
     stroke(255);

     text("Anx",x, y+32); 
     text(total_anx,x+64, y+32); 
     text("Lev",x, y+64); 
     text(level_chosen,x+64, y+64); 
     text("Pic",x, y+96); 
     text(image_chosen,x+64, y+96); 

}

void draw_groupanx(int x, int y)
{
     fill(0);
     stroke(0);
     rectMode(CORNER);
     rect( x, y, 178, 120);
     textSize(32);
     fill(255);
     stroke(255);

     text("Group",x, y+32); 
     text("Anxiety",x, y+64); 
     textSize(48);
     text(total_anx,x+120, y+48); 

     textSize(12);
     text(STRESS_SCALING,x+120, y+96); 


}

void draw_legend(int x, int y)
{
     fill(0);
     stroke(0);
     rectMode(CORNER);
     rect( x, y, 178, 120);
     textSize(16);
 
     stroke(0);
     fill(255,100,100);   //heart BPM
     rect( x, y, 20, 20);
     fill(255);
     stroke(255);
     text("Heart Rate",x+25, y+16); 

     stroke(0);
     fill(green); //movement (accelerations)
     rect( x, y+25, 20, 20);
     fill(255);
     stroke(255);
     text("Movement",x+25, y+25+16); 

     stroke(0);
     fill(0,200,200);   //temperature
     rect( x, y+50, 20, 20);
     fill(255);
     stroke(255);
     text("Temperature",x+25, y+50+16); 

     stroke(0);
     fill(255,255,255); // skin RESistance
     rect( x, y+75, 20, 20);
     fill(255);
     stroke(255);
     text("Skin Resistance",x+25, y+75+16); 
}

void read_sensor(int theID)
{
      raw_packetid[theID] = dat[1];
      raw_bpm[theID] = dat[2];
      raw_mov[theID] = dat[3];
      raw_bat[theID] = dat[4];
      raw_temp[theID] = dat[5];
      raw_gsr[theID] = dat[6];
}


void map_sensors(int theID)
{
   // map each sensor into an anx level from 1 - 9
   
   //heart BPM
   if(raw_bpm[theID] < 70) mapped_bpm[theID] = 1;
   else if(raw_bpm[theID] > 120) mapped_bpm[theID] = 10;
   else mapped_bpm[theID] = (int)map(raw_bpm[theID],70,120,1,10);
   
   //movement
   if(raw_mov[theID] < 1) mapped_mov[theID] = 1;
   else if(raw_mov[theID] > 200) mapped_mov[theID] = 10;
   else mapped_mov[theID] =  (int)map(raw_mov[theID],1,200,1,10);
   
   //temperature
   if(raw_temp[theID] < 2400) mapped_temp[theID] = 1;
   else if(raw_temp[theID] > 2700) mapped_temp[theID] = 10;
   else mapped_temp[theID] =  (int)map(raw_temp[theID],2400,2700,1,10);
   
   //skin resistance
   if(raw_gsr[theID] < 50) mapped_gsr[theID] = 10;
   else if(raw_gsr[theID] > 512) mapped_gsr[theID] = 1;
   else mapped_gsr[theID] =  (int)map(raw_gsr[theID],50,512,10,1);

/*
   print(raw_temp[theID]);
   print("  bpm:");

   print(mapped_bpm[theID]);
   print("  mov:");
   
   print(mapped_mov[theID]);
   print("  temp:");
   print(mapped_temp[theID]);
   print("  gsr:");
   println(mapped_gsr[theID]);
  */
}


void plot_anx_graphs(int theID,int theDataPos)
{
     // convert to graphs
   
   stroke(red);   //heart BPM
   point(theDataPos, (theID*100)+100 - mapped_bpm[theID] * 9); 
   stroke(green); //movement (accelerations)
   point(theDataPos,  (theID*100)+100  - mapped_mov[theID] * 9); 

   //dat[4] is battery level
   
   stroke(200,200,200);   //temperature
   point(theDataPos,  (theID*100)+100  - mapped_temp[theID] * 9); 
   stroke(purple); // skin RESistance
   point(theDataPos,  (theID*100)+100  - mapped_gsr[theID] * 9); 

}

void load_new_group_image(int anx_lev)
{ 
  level_chosen = anx_lev;
  if(level_chosen >  10) 
    level_chosen =  10;

  if(DEBUG)
  {
    print("loading image - lev:");  
    print(level_chosen);  
    print("  pic:");
    println(image_chosen);  
  }
  
 //print("Level chosen is ");
 //print(level_chosen);
 if(level_chosen == 0)
   level_chosen = 1;

  // Test each directory to see if any images exist for this anx level
  // if no images exist
  String chosen_dir_name = null;
  
  //if(directory_chosen == 0)
  //  chosen_dir_name = "Documentary";
  //else if(directory_chosen == 1)
  //  chosen_dir_name = "Medical";  
  //else if (directory_chosen == 2)
  //  chosen_dir_name = "Statistical"; 
  // else if (directory_chosen == 3)
  //  chosen_dir_name = "Number"; 
  //chosen_dir_name = "All";   
  chosen_dir_name = "FinalSet";   
  chosen_dir_name += "/Level" + level_chosen;

  // pick a random number between 0 and the maximum noof images in the Level directory
  int max_image_no = dir_noof_images[level_chosen - 1] - 1;
  
  image_chosen = int(random(max_image_no));
  
  String filename = chosen_dir_name + "/p" + image_chosen + ".jpg";
  File f = new File(dataPath(filename));
  if (f.exists())
  {
      theimg = loadImage( filename );  
      delay(IMAGE_DELAY);
      // if(theimg[imgflag] != null)
      //   image(theimg, IMAGE_X, IMAGE_Y, IMAGE_WIDTH, IMAGE_HEIGHT);  
  } 
}



void serialEvent(Serial p)
{
  inString = p.readString();
}

void draw_graph_bounds()
{
  textSize(18);
  for(int id = 0; id < MAX_PERFORMERS; id++)
  {
//    stroke(255);
//    fill(255);
    int ypos = id * IND_GRAPH_HEIGHT + IND_GRAPH_HEIGHT + 1;  // +1 means data wont be plotted over the axes
//    line(0,ypos,GRAPHS_WIDTH,ypos);
    stroke(255);
    fill(255);
    text(performer_names[id],0,ypos-IND_GRAPH_HEIGHT+20);
  }
}

void plot_raw_data(int theID)
{
   // convert to graphs
   
   stroke(255,100,100);   //heart BPM
   point(graphplot_x, (theID*IND_GRAPH_HEIGHT)+IND_GRAPH_HEIGHT - raw_bpm[theID]/4); 
   point(graphplot_x, (theID*IND_GRAPH_HEIGHT)+IND_GRAPH_HEIGHT - raw_bpm[theID]/4 + 1); 
   stroke(green); //movement (accelerations)
   point(graphplot_x,  (theID*IND_GRAPH_HEIGHT)+IND_GRAPH_HEIGHT  - raw_mov[theID]/12); 
   point(graphplot_x,  (theID*IND_GRAPH_HEIGHT)+IND_GRAPH_HEIGHT  - raw_mov[theID]/12 + 1); 

   //dat[4] is battery level
   
   stroke(0,200,200);   //temperature
   point(graphplot_x,  (theID*IND_GRAPH_HEIGHT)+IND_GRAPH_HEIGHT  - raw_temp[theID]/100); 
   point(graphplot_x,  (theID*IND_GRAPH_HEIGHT)+IND_GRAPH_HEIGHT  - raw_temp[theID]/100 + 1); 
   stroke(255,255,255); // skin RESistance
   point(graphplot_x,  (theID*IND_GRAPH_HEIGHT)+IND_GRAPH_HEIGHT  - raw_gsr[theID]/10); 
   point(graphplot_x,  (theID*IND_GRAPH_HEIGHT)+IND_GRAPH_HEIGHT  - raw_gsr[theID]/10 + 1); 

   
} 
 