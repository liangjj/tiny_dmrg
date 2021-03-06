/**
 * @file exceptions.h
 * @brief A file with a class for DMRG exceptions
 *
 * @author Roger Melko 
 * @author Ivan Gonzalez
 * @date $Date$
 *
 * $Revision$ 
 *
 * This file defines a class for exceptions derived from std::logic_error
 * to be used anywhere in the code
 */
#ifndef EXCEPTION_H
#define EXCEPTION_H

#include<stdexcept>

namespace dmrg{
    /**
     * @brief A class for exceptions to be thrown by our code
     *
     * We derived our exception from std::logic_error and not other
     * std::exception for no reason.
     *
     * @see Josuttis C++ Standard Library (sec 3.3)
     */
    class Exception : public std::logic_error {

	public:

	    /**
	     * brief Constructor
	     *
	     * @param whatString a string with the message the exception
	     * will print out if raised.
	     */
	    Exception(const std::string& whatString) 
		: std::logic_error(whatString) {}
    };
} //namespace dmrg
#endif // EXCEPTION_H
