/*!
 * \file 
 * \author Matthias Wiesenberger
 * \date 20.10.2012 
 */
#ifndef _MESSAGE_
#define _MESSAGE_

#include <iostream>
/*! for the simplified construction of a Message use this Macro*/
#define _ping_ __FILE__, __LINE__ 

namespace spectral
{

/*! @brief class intended for the use in throw statements
 *
 * @ingroup exceptions
 * The objects of this class store a message (that describes the error when thrown)
 * that can then be displayed in a catch block
 * \code
 * try{ throw Message("This is an error!\n", _ping_);}
 * catch( Message& m) {m.display();}
 * \endcode
 */
class Message
{
  private:
    const char* m;
    const char* f;
    const int l;
  public:
    /*! @brief Constructor
     *
     * @param message A character string containing the message
     * @param file The file in which the exception is thrown (contained in the predefined Macro __FILE__)
     * @param line The line in which the exception is thrown (contained in the predefined Macro __LINE__)
     * \note The Macro _ping_ combines __FILE__, __LINE__ in one. 
     */
    Message(const char* message, const char* file, const int line): m(message), f(file), l(line){}
    /*! @brief prints file, line and message to std::cerr
     *
     * It is virtual so that derived classes, that store more Information can display these too.
     */ 
    virtual void display() const
    {
        std::cerr << "Message from file "<<f<<" in line " <<l<<": "<<std::endl
                  << m<<std::endl;
    }
};
} //namespace spectral

#endif // _MESSAGE_
