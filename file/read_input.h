#ifndef _TL_READ_INPUT_
#define _TL_READ_INPUT_

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "../spectral/message.h"

namespace file{

/*! @brief Extract paramters from a file 
 *
 * A parameter is recognised by the character '=' directly(!) in front of it. 
 * All other characters are ignored!
 * @param filename 
 *   The name of the file to be searched for parameters
 * @return 
 *   A vector containing all parameters read
 *   the zeroth element is -1, the first contains the first parameter
 *   so the size is '# of parameters read' + 1
 */
std::vector<double> read_input(const char * filename)
{ 
    char s;
    std::vector<double> para; 
    para.push_back( -1.);
    double read;
    std::ifstream is( filename);
    if( is.fail()) //fail() returns true when either eof is reached or failbit or badbit is set
    {
        std::stringstream s;
        s << "Error opening file '" << filename << "'";
        throw spectral::Message( s.str().c_str() , _ping_);
    }
    while(is.good())
    {
        is.get( s);
        if( s == '=') 
        {
            is >> read;
            para.push_back(read); 
            if( is.fail()) //check if read was successful
                throw spectral::Message( "Error while reading parameter. Is there a character behind = ?\n", _ping_);
        }
        if( is.bad()) //check streams integrity
            throw spectral::Message( "Error while reading file. File corrupted\n", _ping_);
    }
    is.close();
    return para;
}


/**
 * @brief Read a file into a std::string object
 *
 * @param filename The file to read
 *
 * @return file as a string
 */
std::string read_file( const char* filename)
{
    std::ifstream is( filename, std::ios::in | std::ios::binary);
    if( is.fail()) //fail() returns true when either eof is reached or failbit or badbit is set
    {
        std::stringstream s;
        s << "Error opening file '" << filename << "'";
        throw spectral::Message( s.str().c_str() , _ping_);
    }
    std::string s; 
    is.seekg( 0, std::ios::end); //go to end
    s.resize( is.tellg());
    is.seekg( 0, std::ios::beg); //go back to beginning
    is.read( &s[0], s.size());
    is.close();
    return s;

}

/*! @brief Extract paramters from a string
 *
 * A parameter is recognised by the character '=' directly(!) in front of it. 
 * All other characters are ignored!
 * @param file 
 *   The string to be searched for parameters
 * @return 
 *   A vector containing all parameters read
 *   the zeroth element is -1, the first contains the first parameter
 *   so the size is '# of parameters read' + 1
 */
std::vector<double> read_input( const std::string& file)
{
    std::vector<double> para; 
    para.push_back( -1.);
    double read; 
    std::stringstream is( file.data());
    is.seekg(0, std::ios::beg);
    while( is.good())
    {
        char s;
        is.get( s);
        if( s == '=') 
        {
            is >> read;
            para.push_back(read); 
            if( is.fail()) //check if read was successful
                throw spectral::Message( "Error while reading parameter. Is there a character behind = ?\n", _ping_);
        }
        if( is.bad()) //check streams integrity
            throw spectral::Message( "Error while reading file. File corrupted\n", _ping_);
    }
    return para;
}

} //namespace spectral

#endif // _TL_READ_INPUT_
