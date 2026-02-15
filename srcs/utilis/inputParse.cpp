/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   inputParse.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbogovic <dbogovic@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/07 13:43:09 by dbogovic          #+#    #+#             */
/*   Updated: 2026/02/07 13:46:21 by dbogovic         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/main.hpp"
#include <stdexcept>
#include <cstdlib>
#include <climits>

void argcCheck(int argc)
{
	if (argc != 3)
		throw (std::runtime_error("Error: ivalid amount of arguments."));
}

int portParse(char* portStr)
{
	char *endptr;
	long portLong = std::strtol(portStr, &endptr, 10);
	if (*endptr != '\0')
		throw (std::runtime_error("Error: port contains non-numeric characters!"));
	if (portLong == LONG_MAX || portLong == LONG_MIN)
		throw (std::runtime_error("Error: port number out of range!"));
	if (portLong < 1024 || portLong > 65535)
		throw(std::runtime_error("Error: port number is outside accepted range! (Range: 1024 - 65535)"));
	return (static_cast<int>(portLong));
}

std::string passParse(std::string passStr)
{
	if (passStr.empty() || passStr.length() > 32)
		throw (std::runtime_error("Error: bad password! (Password should be between 1-32 characters long)"));
	for (size_t i = 0; i < passStr.length(); ++i)
	{
		if (std::isspace(passStr[i]))
			throw (std::runtime_error("Error: whitespace not allowed in password!"));
	}
	return (passStr);
}
