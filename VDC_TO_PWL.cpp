// VDC_TO_PWL.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>

using uint = unsigned int;

struct VCD_VARIABLE_INSTANCE {
    std::string VAR_NAME = "";
    std::string VAR_ID = "";
    std::vector<char> PREV_VOLT;
    uint BYTES_NO = 1;
};

int main( int argc, char* argv[])
{
    std::string EXTRA_VARIABLES_PATH = "EXTRA_VARIABLES.txt";
    std::string VCD_VARIABLES_PATH = "VCD_VARIABLES.txt";
    std::string VCD_PATH = "VCD_FILE.vcd";
    std::string PWLS_PATH = "PWLS/";
    int CURRENT_TIME = 0;
    char TIME_UNIT = -9;
    int TIME_STEP_NEXT = 3;
    int TIME_STEP_PREV = 0;
    uint MAX_NO_OF_BITS = UINT_MAX;
    int VOLT_HIGH = 5;
    int VOLT_LOW = -5;


    if ( argc >= 2 ) {  //  COMMAND LINE ARGUMENTS FILE PATHS
        VCD_VARIABLES_PATH = argv[1];
    }
    if ( argc >= 3 ) {
        VCD_PATH = argv[2];
    }
    if ( argc >= 4 ) {
        PWLS_PATH = argv[3];
    }
    if ( argc >= 5 ) {
        EXTRA_VARIABLES_PATH = argv[4];
    }


    std::ifstream EXTRA_VARIABLES_FILE(EXTRA_VARIABLES_PATH);
    int EXTRA_VARIABLES_COUNTER = 0;
    std::string file_line = ""; //  USED IN ALL FILES TO READ LINES

    if (!EXTRA_VARIABLES_FILE.is_open()) {
        std::cout << "Could not open extra variables file:    " << EXTRA_VARIABLES_PATH << "\n";
        return -1;
    }

    while (getline(EXTRA_VARIABLES_FILE, file_line)) {  //  PARSE EXTRA VARIABLES FILE. WE USE THE COUNTER TO DETERMINE WHICH VARIABLE TO INITIALIAZE

        std::stringstream sstream(file_line);
        std::string word_from_line = "";

        getline(sstream, word_from_line, ' ');

        if ( EXTRA_VARIABLES_COUNTER == 0 ) {
            CURRENT_TIME = std::stoi( word_from_line );
        }
        else if ( EXTRA_VARIABLES_COUNTER == 1 ) {
            TIME_UNIT = char( std::stoi( word_from_line ) );
        }
        else if ( EXTRA_VARIABLES_COUNTER == 2 ) {
            TIME_STEP_NEXT = std::stoi( word_from_line );
        }
        else if ( EXTRA_VARIABLES_COUNTER == 3 ) {
            TIME_STEP_PREV = std::stoi( word_from_line );
        }
        else if ( EXTRA_VARIABLES_COUNTER == 4 ) {
            MAX_NO_OF_BITS = uint( std::atoll( word_from_line.c_str() ) );
        }
        else if ( EXTRA_VARIABLES_COUNTER == 5 ) {
            VOLT_HIGH = std::stoi( word_from_line );
        }
        else if ( EXTRA_VARIABLES_COUNTER == 6 ) {
            VOLT_LOW = std::stoi( word_from_line );
        }

        EXTRA_VARIABLES_COUNTER++;

     }

    EXTRA_VARIABLES_FILE.close();



    std::vector< VCD_VARIABLE_INSTANCE > VCD_VARS;  //  HOLDS VCD SIGNAL NAME, IDS, SIZE...
    std::vector< std::string > DEST_FILE;   //  HOLDS PWL FILE NAMES
    
    std::ifstream VCD_VARIABLES_FILE(VCD_VARIABLES_PATH);

    if (!VCD_VARIABLES_FILE.is_open()) {
        std::cout << "Could not open variables file:    " << VCD_VARIABLES_PATH << "\n";
        return -2;
    }

    while (getline(VCD_VARIABLES_FILE, file_line)) {    //  PARSE THE VCD VARIABLES FILE TO INITIALIZE THE VCD SIGNALS WE ARE LOOKING FOR AND THE PWLS

        std::stringstream sstream(file_line);
        std::string word_from_line = "";

        VCD_VARIABLE_INSTANCE TEMP_VCD;
        getline(sstream, TEMP_VCD.VAR_NAME, ' ');
        VCD_VARS.emplace_back( TEMP_VCD );

        getline(sstream, word_from_line, ' ');
        DEST_FILE.emplace_back( word_from_line );

     }

    VCD_VARIABLES_FILE.close();


    std::ifstream VCD_FILE(VCD_PATH);

    if (!VCD_FILE.is_open()) {
        std::cout << "Could not open VCD file:    " << VCD_PATH << "\n";
        return -3;
    }

    while (getline(VCD_FILE, file_line)) {  //  PARSE VCD FILE TO WRITE DOWN SIGNAL IDS AND SIZE
        if ( file_line == "$enddefinitions $end" ) break;   //  STOP AT SIGNAL DECLERATION END

        for ( uint i=0; i < VCD_VARS.size(); i++ ) {
            if ( file_line.find( VCD_VARS[i].VAR_NAME ) != std::string::npos ) {
                
                std::stringstream sstream(file_line);
                std::string word_from_line = "";
                char word_index = 0;

                while (getline(sstream, word_from_line, ' ')) {
                    if ( word_index == 2 ) {    //  THIRD WORD HOLDS THE SIGNAL SIZE. IF IT EXCEEDS THE MAXIMUM WE TRIM IT
                        VCD_VARS[i].BYTES_NO = stoi( word_from_line );
                        if ( VCD_VARS[i].BYTES_NO >= MAX_NO_OF_BITS ) VCD_VARS[i].BYTES_NO = MAX_NO_OF_BITS;
                        VCD_VARS[i].PREV_VOLT = std::vector<char>( VCD_VARS[i].BYTES_NO, VOLT_LOW );
                    }
                    else if ( word_index == 3 ) {   //  FORTH WORD HOLDS THE SIGNAL ID
                        VCD_VARS[i].VAR_ID = word_from_line;
                        break;
                    }
                    word_index++;
                }

                continue;
            }
        }

     }


    std::vector< std::vector< std::unique_ptr<std::ofstream> > > PWL_FILES; //  2D POINTER VECTOR FOR PWL. HAS TO BE 2D SINCE SOME SIGNALS MAY HAVE MORE THAN ONE BIT
    for ( uint i=0; i < DEST_FILE.size(); i++ ) {

        PWL_FILES.emplace_back( std::vector< std::unique_ptr<std::ofstream> >() );

        for (uint y = 0; y < VCD_VARS[i].BYTES_NO; y++) {   //  ONE BIT SIGNALS WILL REPEAT ONLY ONCE

            std::string BIT_ENUMERATION = ""; // WE ONLY ENUMERATE FILES WITH THE BIT NUMBER IF SIGNAL HAS MORE THATN ONE BITS
            if ( VCD_VARS[i].BYTES_NO > 1 ) BIT_ENUMERATION =  "_BIT_" + std::to_string(y);

            PWL_FILES[i].emplace_back(std::make_unique<std::ofstream>(PWLS_PATH + DEST_FILE[i] + BIT_ENUMERATION + ".txt"));    //  CREATE POINTER TO PWL FILE

            if (!PWL_FILES[i][0]->is_open()) {
                std::cout << "Could not open PWL file ( " + std::to_string(i) + ", " + std::to_string(y) + " ):    " << PWLS_PATH + DEST_FILE[i] + "_BIT_" + std::to_string(y) + ".txt" << "\n";
                return -3;
            }

            *PWL_FILES[i][y] << "0 " + std::to_string( VOLT_LOW ); //  INITIALIZE EACH FILE AT TIME ZERO WITH LOW VOLTAGE
        }
    }




    while (getline(VCD_FILE, file_line)) {  //  PARSE THE REST OF THE VCD FILE 
        
        if ( file_line == "$dumpvars" ) continue;

        if ( file_line[0] == '#' ) {    //  CHANGE CURRENT TIMESTAMP ONLY OF TIME IS LARGER THAN THE ONE GIVEN (WE CAN INTIALIZE THE STARTING TIME TO WHATEVER WE WANT)
            if ( std::stoi( file_line.substr(1) ) > CURRENT_TIME ) CURRENT_TIME = std::stoi( file_line.substr(1) );
            continue;
        }

        std::stringstream sstream(file_line);
        std::string word_from_line_value = "";
        std::string word_from_line_ID = "";

        if ( file_line[0] == 'b' ) {    //  SEPERATE SIGNAL AND ID BASED ON WETHER IT HAS MANY OR ONE BITS
            getline(sstream, word_from_line_value, ' ');
            getline(sstream, word_from_line_ID, ' ');
        }
        else {
            word_from_line_value = file_line[0];
            word_from_line_ID = file_line.substr(1);
        }

        for ( uint i = 0; i < VCD_VARS.size(); i++ ) {  //  COMPARE GIVEN ID WITH EACH VARIABLE ID WE ARE LOOKING FOR
            if (  word_from_line_ID != VCD_VARS[i].VAR_ID ) continue;

            if ( word_from_line_value[0] == 'b' ) { //  IF SIGNAL HAS MORE THAN ONE BIT, REMOVE THE STARTING 'b' AND FORMAT THE SIGNAL VALUE TO THE MAXIMUM GIVEN SIZE
                word_from_line_value = word_from_line_value.substr(1);

                if ( VCD_VARS[i].BYTES_NO > word_from_line_value.size() ) word_from_line_value = std::string( VCD_VARS[i].BYTES_NO - word_from_line_value.size(), '0' ) + word_from_line_value;
            }

            for ( int y = VCD_VARS[i].BYTES_NO-1; y > -1; y-- ) {   //  WRITE DOWN THE TIME OF CHANGE FOR EACH BIT. RUNS ONCE FOR ONE BIT SIGNALS

                int NEW_VOLT = 0;   //  CALCULATE IF THE CURRENT BIT RESPONDS TO HIGH OR LOW VOLTAGE
                if ( word_from_line_value[y] == '1' ) NEW_VOLT = VOLT_HIGH;
                else NEW_VOLT = VOLT_LOW;

                if ( VCD_VARS[i].PREV_VOLT[y] == NEW_VOLT ) continue;   //  IF THE BIT'S SIGNAL IS THE SAME AS BEFORE WE SKIP 

                //  WRITE DOWN THE BEGINING OF THE SIGNAL CHANGE AND ITS END
                *PWL_FILES[i][VCD_VARS[i].BYTES_NO-1 - y] << "\n" + std::to_string( CURRENT_TIME - TIME_STEP_PREV ) + "e-9 " +  std::to_string( VCD_VARS[i].PREV_VOLT[y] );
                *PWL_FILES[i][VCD_VARS[i].BYTES_NO-1 - y] << "\n" + std::to_string( CURRENT_TIME + TIME_STEP_NEXT ) + "e-9 " +  std::to_string( NEW_VOLT );

                VCD_VARS[i].PREV_VOLT[y] = NEW_VOLT;    //  CHANGE THE BIT'S VOLTAGE SO IT CAN BE COMPARED WITH FUTURE CHANGES

            }

            break;
        }

    }



    VCD_FILE.close();   //  CLOSE REMAINING FILES

    for ( uint i = 0; i < PWL_FILES.size(); i++ ) {
        for ( uint y = 0; y < PWL_FILES[i].size(); y++ ) {
            PWL_FILES[i][y]->close();
        }
    }
    
}