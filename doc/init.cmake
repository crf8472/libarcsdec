## Initialize DOC_GEN_DIR.
##
## Reason for this script is to make 'rm -rf generated-docs && make doc' work.
## We have to re-generate 'generated-docs' automatically once it was removed.
## (Having file/MAKE_DIRECTORY in CMakeLists.txt just generates the directory
## once at configuration time.)


## --- Generate documentation output directory

message (STATUS "Ensure documentation build directory '${DOC_GEN_DIR}' exists" )
file (MAKE_DIRECTORY ${DOC_GEN_DIR} )

