/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <typeindex>

#include "FunctionIO.hpp"

std::string functionio_get_type_name(std::type_index tindex)
{
    if (       std::type_index(typeid(bool)) == tindex) {
        return "bool";
    } else if (std::type_index(typeid(char)) == tindex) {
        return "char";
    } else if (std::type_index(typeid(unsigned)) == tindex) {
        return "unsigned";
    } else if (std::type_index(typeid(int)) == tindex) {
        return "int";
    } else if (std::type_index(typeid(float)) == tindex) {
        return "float";
    } else if (std::type_index(typeid(double)) == tindex) {
        return "double";
    } else
        return "unknown";
}