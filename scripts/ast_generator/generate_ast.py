#!/usr/bin/python

"""A lox lang abstract syntax tree source generator.

This script will generate a C++ header file containing the definition of an
AST for the lox language. generate_ast.py expects a JSON AST definition file.
See ast.json for a sample definition file. You can visit
https://craftinginterpreters.com/representing-code.html for more information.
"""

import os
import json
import argparse


def ws(num_spaces=4):
    """Return a string with num_spaces spaces."""

    return num_spaces * ' '


def generate_visitor(writer, base_name, visitor_return, subtypes):
    """Write the AST's abstract Visitor class definition."""

    writer.write('class ' + base_name + 'Visitor\n')
    writer.write('{\n')
    writer.write('public:\n')
    for subtype in subtypes:
        writer.write(ws() + 'virtual ' + visitor_return + ' Visit' +
                     subtype['name'] + base_name + '(')
        writer.write(subtype['name'] + '& ' + base_name.lower() + ') = 0;\n')
    writer.write('}; // end Visitor')


def generate_type(writer, base_name, visitor_return, subtype_info):
    """Write the AST's subtype class definition(s)."""

    # Beginning of class definition.
    writer.write('class ' + subtype_info['name'] + ' : public ' + base_name +
                 '\n')
    writer.write('{\n')
    writer.write('public:\n')

    # Define defaults for the special member functions.
    writer.write(ws() + subtype_info['name'] + '() = default;\n')
    writer.write(ws() + '~' + subtype_info['name'] + '() = default;\n')
    writer.write(ws() + subtype_info['name'] + '(const ' +
                 subtype_info['name'] + '&) = default;\n')
    writer.write(ws() + subtype_info['name'] + '& operator=(const ' +
                 subtype_info['name'] + '&) = default;\n')
    writer.write(ws() + subtype_info['name'] + '(' + subtype_info['name'] +
                 '&&) = default;\n')
    writer.write(ws() + subtype_info['name'] + '& operator=(' +
                 subtype_info['name'] + '&&) = default;\n')
    writer.write('\n')

    # Constructor definition.
    writer.write(ws() + subtype_info['name'] + '(')
    fields = []
    for field in subtype_info['fields']:
        fields.append(field['type'] + ' ' + field['name'] + '_')
    writer.write(', '.join(fields) + ') : \n')

    # Constructor definition: the initializer list.
    init_list = [8 * ' ' + field['name'] + '(' + field['name'] + '_' + ')'
                 for field in subtype_info['fields']]
    writer.write(',\n'.join(init_list))
    writer.write('\n' + ws() + '{\n')
    writer.write('\n')
    writer.write(ws() + '}\n\n')

    # base_name::Accept method override.
    writer.write(ws() + visitor_return + ' Accept(' + base_name +
                 'Visitor& visitor) final\n')
    writer.write(ws() + '{\n')
    if 'void' == visitor_return:
        writer.write(ws(8) + 'visitor.Visit' + subtype_info['name'] +
                     base_name + '(*this);\n')
    else:
        writer.write(ws(8) + 'return visitor.Visit' + subtype_info['name'] +
                     base_name + '(*this);\n')
    writer.write(ws() + '}\n\n')

    # Class member declarations.
    for field in subtype_info['fields']:
        if field['type'] == base_name:
            writer.write(ws() + 'std::shared_ptr<' + field['type'] + '> ' +
                         field['name'] + ';\n')
        else:
            writer.write(ws() + field['type'] + ' ' + field['name'] + ';\n')

    writer.write('}; // end ' + subtype_info['name'])


def generate_ast(writer, data):
    """Write the AST definition to a C++ header file.

    Parameters
    ----------
    writer : File object to which the source code will be written.
    data   : JSON definition of AST types and fields.
    """

    base_name = data['base_name']
    visitor_return = data['visitor_return']

    writer.write('/* Autogenerated using generate_ast.py, DO NOT EDIT. */\n')
    writer.write('#pragma once\n\n')

    # Include directives.
    for header in data['headers']:
        writer.write('#include ' + header + '\n')

    # Namespace declaration.
    writer.write('\n')
    writer.write('namespace lox\n')
    writer.write('{\n')
    writer.write('namespace ast\n')
    writer.write('{\n')

    # Forward declare the subtype classes.
    for subtype in data['subtypes']:
        writer.write('class ' + subtype['name'] + ';\n')

    # Write the Visitor abstract class.
    writer.write('\n')
    generate_visitor(writer, base_name, visitor_return, data['subtypes'])
    writer.write('\n\n')

    # Write the base class definition.
    writer.write('class ' + base_name + '\n')
    writer.write('{\n')
    writer.write('public:\n')
    writer.write(ws() + 'virtual ~' + base_name + '() = default;\n')
    writer.write(ws() + 'virtual ' + visitor_return +
                 ' Accept(' + base_name + 'Visitor& visitor) = 0;' + '\n')
    writer.write('}; // end ' + base_name)
    writer.write('\n')

    # Write the definition for each subtype.
    for subtype in data['subtypes']:
        writer.write('\n')
        generate_type(writer, base_name, visitor_return, subtype)
        writer.write('\n')

    writer.write('} // end ast\n')
    writer.write('} // end lox')


if '__main__' == __name__:
    parser = argparse.ArgumentParser(description='Generate AST source')
    parser.add_argument('-a', '--ast-def', default='ast.json', type=str,
                        help='AST JSON definition file')
    parser.add_argument('-o', '--output-dir', default='.', type=str,
                        help='directory to which source files will be output')
    args = parser.parse_args()

    with open(args.ast_def, 'r') as ast_def_file:
        data = json.load(ast_def_file)
        for ast in data['ast']:
            header_path = os.path.join(args.output_dir, ast['base_name'] +
                                       '.h')
            with open(header_path, 'w') as writer:
                generate_ast(writer, ast)
