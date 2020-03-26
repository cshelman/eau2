### Intro

The idea behind this kind of serialization was to make it so any data type could be sent across a wire, and be instantly recognizable to a de-serializer on the other side, while at the same time being human readable.

Currently the data types are explicitly de-serialized, however, it is trivial to de-serialize based off of a given serialized char* as the leading tag will always tell you what to de-serialize.

### Overview of serial.h

A Buffer is like an ArrayList of chars, except it can only be added to. The buffer was defined to act as a flexible accumulator for the resultant char bytes.

The idea of the serialization was to build up from serializing primitive types into the more complicated classes, so each high level data structure could simply call the low level ones to serialize their pieces.

The three functions at the top of serial.h act as utility functions, and are used by many/all of the serialization/de-serialization functions.

    The functions add_tag() and get_word() are inverses: add_tag() adds the tag (char, int, etc.) and braces around the serialized data, while get_word() removes the tag and braces. At the moment get_word() does not allow for serialized data to contain double quote characters.

    The function get_components() is used to retrieve the fields of the more complex classes being de-serialized. It returns a custom StringArray which behaves exactly as you would expect.

### Current Issues

The serialization of arrays (or any variable length data structure) is unfinished. Currently it is repeated code within the serialize and de-serialize classes that use it, but with a little tweaking it could easily be abstracted.

The get_word() and get_components() functions could be improved both in readability and in speed, as they iterate through the serialized data character by character.

Serialization and de-serialization of messages should also be abstracted, as the functions have grown too large to be maintainable and contain some repeated code.

### Running tests

Tests are kept in the /test/ directory, and print the serialized then de-serialized data.
The main function runs similar code, but does not print the results.

Use command `make test` to run all tests.
Use commands: `make test_string_array`
              `make test_double_array`
              `make test_message`
              to run only the tests which build up to each complex class.