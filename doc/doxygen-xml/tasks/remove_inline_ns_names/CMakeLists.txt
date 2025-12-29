## CMake script for removing inline namespace names doxygen's class*.xml files
##
## vim:fdm=marker

cmake_minimum_required (VERSION 3.12 )

function (register_remove_inline_ns_names XML_INPUT_PATH )

	message (STATUS "Add doxygen Task: remove inline namespace names" )

	## Note that '_1_1' denotes '::' and each original underscore within a
	## namespace name is escaped with another underscore, thus
	## 'arcsdec::v_1_0_0::' becomes 'arcsdec_1_1v__1__0__0_1_1'
	set (_sep   "_1_1" )
	set (_v100 "arcsdec${_sep}v__1__0__0${_sep}" )

	## For template classes
	set (_langle "_3_01" )
	set (_rangle "_01_4" )
	set (_ttype  "T_00_01" )
	set (_tf     "F_00_01" )

	set (_XML_CLASSFILES
		## Classes
		## Listed in tab 'Classes':
		"${XML_INPUT_PATH}/class${_v100}ARCSCalculator.xml"
		"${XML_INPUT_PATH}/class${_v100}ARIdCalculator.xml"
		"${XML_INPUT_PATH}/class${_v100}AudioInfo.xml"
		"${XML_INPUT_PATH}/class${_v100}AudioReader.xml"
		"${XML_INPUT_PATH}/class${_v100}AudioReaderImpl.xml"
		"${XML_INPUT_PATH}/class${_v100}AudioValidator.xml"
		"${XML_INPUT_PATH}/class${_v100}Bytes.xml"
		"${XML_INPUT_PATH}/class${_v100}ByteSeq.xml"
		"${XML_INPUT_PATH}/class${_v100}DefaultPreference.xml"
		"${XML_INPUT_PATH}/class${_v100}DefaultSelector.xml"
		"${XML_INPUT_PATH}/class${_v100}DefaultValidator.xml"
		"${XML_INPUT_PATH}/class${_v100}DescriptorPreference.xml"
		"${XML_INPUT_PATH}/class${_v100}FileReader.xml"
		"${XML_INPUT_PATH}/class${_v100}FileReaderDescriptor.xml"
		"${XML_INPUT_PATH}/class${_v100}FileReaderPreferenceSelection.xml"
		"${XML_INPUT_PATH}/class${_v100}FileReaderProvider.xml"
		"${XML_INPUT_PATH}/class${_v100}FileReaderRegistry.xml"
		"${XML_INPUT_PATH}/class${_v100}FileReaderSelection.xml"
		"${XML_INPUT_PATH}/class${_v100}FileReaderSelector.xml"
		"${XML_INPUT_PATH}/class${_v100}FileReadException.xml"
		"${XML_INPUT_PATH}/class${_v100}FormatMatcher.xml"
		"${XML_INPUT_PATH}/class${_v100}FormatPreference.xml"
		"${XML_INPUT_PATH}/class${_v100}IdSelector.xml"
		"${XML_INPUT_PATH}/class${_v100}InputFormatException.xml"
		"${XML_INPUT_PATH}/class${_v100}InvalidAudioException.xml"
		"${XML_INPUT_PATH}/class${_v100}Matcher.xml"
		"${XML_INPUT_PATH}/class${_v100}MetadataParseException.xml"
		"${XML_INPUT_PATH}/class${_v100}MetadataParser.xml"
		"${XML_INPUT_PATH}/class${_v100}MetadataParserImpl.xml"
		"${XML_INPUT_PATH}/class${_v100}MinPreference.xml"
		"${XML_INPUT_PATH}/class${_v100}ReaderAndFormatHolder.xml"
		"${XML_INPUT_PATH}/class${_v100}SampleProcessor.xml"
		"${XML_INPUT_PATH}/class${_v100}SampleProvider.xml"
		"${XML_INPUT_PATH}/class${_v100}SelectionPerformer.xml"
		"${XML_INPUT_PATH}/class${_v100}ToCParser.xml"
		#
		## Structs
		## Listed in tab 'Classes':
		"${XML_INPUT_PATH}/struct${_v100}BigEndianBytes.xml"
		"${XML_INPUT_PATH}/struct${_v100}BLOCKSIZE.xml"
		"${XML_INPUT_PATH}/struct${_v100}CDDAValidator.xml"
		"${XML_INPUT_PATH}/struct${_v100}LittleEndianBytes.xml"
		"${XML_INPUT_PATH}/struct${_v100}RegisterDescriptor.xml"
		"${XML_INPUT_PATH}/struct${_v100}RegisterFormat.xml"
		## Not required for success:
		#"${XML_INPUT_PATH}/struct${_v100}Comparable.xml"
		#"${XML_INPUT_PATH}/struct${_v100}details${_sep}ci__chair__traits.xml"
		#"${XML_INPUT_PATH}/struct${_v100}details${_sep}CreateReader.xml"
		#"${XML_INPUT_PATH}/struct${_v100}details${_sep}signedness.xml"
	)

	add_doxygen_xslt_task ("remove_inline_ns_names"
		"${_XML_CLASSFILES}" "${XML_INPUT_PATH}" )

	unset (_ttype )
	unset (_tf )
	unset (_rangle )
	unset (_langle )
	unset (_v100 )
	unset (_sep )

	unset (_XML_CLASSFILES )

	## TODO May be we can glob exactly the relevant files?
	## Like:
	##
	## file (GLOB _XMLFILES
	##	"${XML_INPUT_PATH}/structarcsdec*.xml"
	##	"${XML_INPUT_PATH}/classarcsdec*.xml")
	##
	## But at the time when this function is executed, they are not yet created.

endfunction()

