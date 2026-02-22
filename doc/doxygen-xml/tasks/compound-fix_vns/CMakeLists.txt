## CMake script for inserting missing inline namespace to some compounds' names
##
## vim:fdm=marker

cmake_minimum_required (VERSION 3.12 )

function (register_compound-fix_vns XML_INPUT_PATH )

	message (STATUS
		"Add doxygen Task: fix inline namespaces in compound names" )

	set (_sep_     "_1_1" )
	set (_arcsdec_ "arcsdec")
	set (_v100_    "v__1__0__0" )
	set (_arcsdec_v100_ "${_arcsdec_}${_sep_}${_v100_}${_sep_}" )

	## refids of the compounds to be fixed.
	## Only the compounds in the Classes/'annotated' tab are considered.
	set (_XML_COMPOUNDS
		"class${_arcsdec_v100_}ARCSCalculator" ;
		"class${_arcsdec_v100_}ARIdCalculator" ;
		"class${_arcsdec_v100_}AudioInfo" ;
		"class${_arcsdec_v100_}AudioReader" ;
		"class${_arcsdec_v100_}AudioReaderImpl" ;
		"class${_arcsdec_v100_}AudioValidator" ;
		"class${_arcsdec_v100_}Bytes" ;
		"class${_arcsdec_v100_}ByteSeq" ;
		"class${_arcsdec_v100_}DefaultPreference" ;
		"class${_arcsdec_v100_}DefaultSelector" ;
		"class${_arcsdec_v100_}DefaultValidator" ;
		"class${_arcsdec_v100_}DescriptorPreference" ;
		"class${_arcsdec_v100_}FileReader" ;
		"class${_arcsdec_v100_}FileReaderDescriptor" ;
		"class${_arcsdec_v100_}FileReaderPreferenceSelection" ;
		"class${_arcsdec_v100_}FileReaderProvider" ;
		"class${_arcsdec_v100_}FileReaderRegistry" ;
		"class${_arcsdec_v100_}FileReaderSelection" ;
		"class${_arcsdec_v100_}FileReaderSelector" ;
		"class${_arcsdec_v100_}FileReadException" ;
		"class${_arcsdec_v100_}FormatMatcher" ;
		"class${_arcsdec_v100_}FormatPreference" ;
		"class${_arcsdec_v100_}IdSelector" ;
		"class${_arcsdec_v100_}InputFormatException" ;
		"class${_arcsdec_v100_}InvalidAudioException" ;
		"class${_arcsdec_v100_}Matcher" ;
		"class${_arcsdec_v100_}MetadataParseException" ;
		"class${_arcsdec_v100_}MetadataParser" ;
		"class${_arcsdec_v100_}MetadataParserImpl" ;
		"class${_arcsdec_v100_}MinPreference" ;
		"class${_arcsdec_v100_}ReaderAndFormatHolder" ;
		"class${_arcsdec_v100_}SampleProcessor" ;
		"class${_arcsdec_v100_}SampleProvider" ;
		"class${_arcsdec_v100_}SelectionPerformer" ;
		"class${_arcsdec_v100_}ToCParser" ;
		#
		"struct${_arcsdec_v100_}BigEndianBytes" ;
		"struct${_arcsdec_v100_}BLOCKSIZE" ;
		"struct${_arcsdec_v100_}CDDAValidator" ;
		"struct${_arcsdec_v100_}LittleEndianBytes" ;
		"struct${_arcsdec_v100_}RegisterDescriptor" ;
		"struct${_arcsdec_v100_}RegisterFormat" ;
	)

	## Expand the compounds' refids to XML filenames
	list (TRANSFORM _XML_COMPOUNDS REPLACE "(.+)" "${XML_INPUT_PATH}/\\1.xml" )

	add_doxygen_xslt_task ("compound-fix_vns" "${_XML_COMPOUNDS}"
		"${XML_INPUT_PATH}" )

	unset (_XML_COMPOUNDS )
	unset (_sep_ )
	unset (_arcsdec_ )
	unset (_v100_ )
	unset (_arcsdec_v100_ )

endfunction()

