/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbogovic <dbogovic@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/07 13:18:35 by dbogovic          #+#    #+#             */
/*   Updated: 2026/02/07 19:29:08 by dbogovic         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MAIN_HPP
#define MAIN_HPP

#include <string>
#include <poll.h>


void					argcCheck(int argc);
int						portParse(char* portStr);
std::string 			passParse(std::string passStr);


#endif
