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
	set (_read     "read${_sep_}" )
	set (_select   "select${_sep_}" )
	set (_calc     "calc${_sep_}" )

	## refids of the compounds to be fixed.
	## Only the compounds in the Classes/'annotated' tab are considered.
	set (_XML_COMPOUNDS
		"class${_arcsdec_v100_}${_calc}ARCSCalculator" ;
		"class${_arcsdec_v100_}${_calc}ARIdCalculator" ;
		"class${_arcsdec_v100_}${_calc}AudioInfo" ;
		"class${_arcsdec_v100_}${_read}AudioReader" ;
		"class${_arcsdec_v100_}${_read}AudioReaderImpl" ;
		"class${_arcsdec_v100_}${_read}AudioValidator" ;
		"class${_arcsdec_v100_}${_read}Bytes" ;
		"class${_arcsdec_v100_}${_read}ByteSeq" ;
		"class${_arcsdec_v100_}${_select}DefaultPreference" ;
		"class${_arcsdec_v100_}${_select}DefaultSelector" ;
		"class${_arcsdec_v100_}${_read}DefaultValidator" ;
		"class${_arcsdec_v100_}${_select}DescriptorPreference" ;
		"class${_arcsdec_v100_}${_read}FileReader" ;
		"class${_arcsdec_v100_}${_read}FileReaderDescriptor" ;
		"class${_arcsdec_v100_}${_select}FileReaderPreferenceSelection" ;
		"class${_arcsdec_v100_}${_select}FileReaderProvider" ;
		"class${_arcsdec_v100_}${_select}FileReaderRegistry" ;
		"class${_arcsdec_v100_}${_select}FileReaderSelection" ;
		"class${_arcsdec_v100_}${_select}FileReaderSelector" ;
		"class${_arcsdec_v100_}${_read}FileReadException" ;
		"class${_arcsdec_v100_}${_read}FormatMatcher" ;
		"class${_arcsdec_v100_}${_select}FormatPreference" ;
		"class${_arcsdec_v100_}${_select}IdSelector" ;
		"class${_arcsdec_v100_}${_read}InputFormatException" ;
		"class${_arcsdec_v100_}${_read}InvalidAudioException" ;
		"class${_arcsdec_v100_}${_read}Matcher" ;
		"class${_arcsdec_v100_}${_read}MetadataParseException" ;
		"class${_arcsdec_v100_}${_read}MetadataParser" ;
		"class${_arcsdec_v100_}${_read}MetadataParserImpl" ;
		"class${_arcsdec_v100_}${_select}MinPreference" ;
		"class${_arcsdec_v100_}${_select}ReaderAndFormatHolder" ;
		"class${_arcsdec_v100_}${_read}SampleProcessor" ;
		"class${_arcsdec_v100_}${_read}SampleProvider" ;
		"class${_arcsdec_v100_}${_select}SelectionPerformer" ;
		"class${_arcsdec_v100_}${_calc}ToCParser" ;
		#
		"struct${_arcsdec_v100_}${_read}BigEndianBytes" ;
		"struct${_arcsdec_v100_}${_read}BLOCKSIZE" ;
		"struct${_arcsdec_v100_}${_read}CDDAValidator" ;
		"struct${_arcsdec_v100_}${_read}LittleEndianBytes" ;
		"struct${_arcsdec_v100_}${_select}RegisterDescriptor" ;
		"struct${_arcsdec_v100_}${_select}RegisterFormat" ;
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

