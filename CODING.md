# Coding Style


I describe the coding style I used to use in the project. It's main purpose is a
documentation for myself. It's not comprehensive but enumerates the points where
I decided to reduce the wiggle room for stylistic creativity.

Beware: the following contains my opinion. You might not like it. It does *not*
claim to be the truth to obey for everybody, it is just the current setting for
libarcsdec.



# Names

- Variable naming style is lower_snake_case: ``sample_seq``,
  ``first_idx``, etc.
- Class names are UpperCamelCase: ``AudioReader``, ``ARCSCalculator``, etc.
- Non-public class member variables are suffixed with an underscore:
  ``config_``, ``handler_``, etc.
- Namespace names consist of lower case ASCII-letters (``[a-z]``)
- Macro names are upper case ASCII-letters and may contain underscores
  (``[A-Z_]``)


# Headers, Declarations, Definitions

- Header file names end with ``.hpp`` (not ``.h``,``.hh`` or anything else).
- Headers do not contain definitions, except for template (member) functions or
  for functions the compiler must be able to inline.
- Avoid inline definitions in the body of non-template classes.
- For templates, use ``typename`` for sites where base types are allowed and use
  ``class`` for sites where own class definitions are required.


# Formatting

- Use tabs not spaces for indenting
- 1 indent step is 1 tab = 4 spaces.
- Put opening and closing curly braces of blocks (``{`` and ``}``) on the next
  line.
- There is no space after the opening parenthesis ``(`` and no space before the
  closing parenthesis ``)``.
- Braced initializers: There is one space before and after the opening brace
  ``{`` and one space before the closing brace ``}``.
- There is a space before and after any operator.
- Put the ``= 0;`` of a pure virtual method declaration on a separate line.
- Constructor initialization lists are formatted with one assignment per line,
  with leading ``:`` or ``,`` and indented one step to the constructor name.


# Compiler feedback

- Eliminate *all* compiler warnings for *all* supported compilers from your
  code. (Root ``CMakeLists.txt`` declares the compilers to support.)


# Documentation Comments

- Everything declared in a header, i.e. every namespace, every function,
  every constant, every type, every class and every class member must have a
  documentation comment.
- Every declaration in a source file except implementation classes (i.e.
  subclasses that are ``final`` and not declared in a header) must have a
  documentation comment.
- Implementation classes and their members may or may not have documentation
  comments.
- If something involves a value, make the unit of the value maximally clear
  (good: ``bytes_per_frame``, ``total_samples`` - ``bad: ``frame_size``,
  ``samples``).
- For input, you may also use ``using`` declarations to hint on further
  restrictions of types.
- If something involves an index, make maximally clear whether the index is
  1-based or 0-based or whatever its base may be.
- Use Doxygens ``\todo`` tags sparingly and only for issues that are about to
  change the API, not for mere implementation issues.


# Non-documentation Comments

- The following constructions must have a comment: every improvable code, every
  code that is outcommented and every not improvable hack that has to
  document its indispensability.
- If you have to comment out code or place a commented code snippet, include the
  reasons to do that in the comment.
- If a comment is indicated, prefer commenting blocks over commenting single
  lines. If you comment a block, the comment is preferably as short as possible
  and just summons up what the block is achieving.
- Don't use ``/*`` ``*/`` for commenting out multiple lines. Use ``//`` on
  every line instead.
- If something is wrong but can't be corrected immediately, describe the bug
  leaving a FIXME tag explaining the need at its very site in the code.
- Use ``TODO`` tags within the code generously to mark sites were non-bug,
  non-API issues are to do. For bugs, use ``FIXME``. If unsure, use ``XXX``.


# Character encoding and newlines

- Libarcsdec project files have unix line endings (== line feed,``0x0A``).
  Whatever your git is configured to checkout, ensure that your git commits unix
  line endings.
- Libarcsdec project files are encoded UTF-8. Whatever your git is configured to
  checkout, ensure that your git commits UTF-8 encoded characters.
- Ensure that every source file you modify has a unix line ending (a blank line)
  at the end of the file.

