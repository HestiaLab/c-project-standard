# Generate the WiX component group for the staged template tree.
#
# Invoked at build time by the `msi` target, after install-staging has run.
# Doing this at build time avoids the configure-time glob chicken-and-egg
# problem on a clean checkout. The generated Directory tree preserves the
# template's layout inside share/c-project-standard.

if(NOT DEFINED CPROJECT_STAGING_TEMPLATE_DIR)
    message(FATAL_ERROR "CPROJECT_STAGING_TEMPLATE_DIR is required")
endif()
if(NOT DEFINED CPROJECT_OUTPUT)
    message(FATAL_ERROR "CPROJECT_OUTPUT is required")
endif()
if(NOT IS_DIRECTORY "${CPROJECT_STAGING_TEMPLATE_DIR}")
    message(FATAL_ERROR "Staged template directory does not exist: ${CPROJECT_STAGING_TEMPLATE_DIR}")
endif()

set(_body_file "${CPROJECT_OUTPUT}.body")
set(_refs_file "${CPROJECT_OUTPUT}.refs")
file(WRITE "${_body_file}" "")
file(WRITE "${_refs_file}" "")

get_filename_component(_output_dir "${CPROJECT_OUTPUT}" DIRECTORY)
file(MAKE_DIRECTORY "${_output_dir}")

function(xml_escape value out_var)
    string(REPLACE "&" "&amp;" _value "${value}")
    string(REPLACE "<" "&lt;" _value "${_value}")
    string(REPLACE "\"" "&quot;" _value "${_value}")
    set(${out_var} "${_value}" PARENT_SCOPE)
endfunction()

function(emit_file relative indent)
    get_filename_component(_name "${relative}" NAME)
    string(REGEX REPLACE "[^A-Za-z0-9_]" "_" _identifier "${relative}")
    string(REPLACE "/" "\\" _windows_relative "${relative}")
    xml_escape("${_windows_relative}" _windows_relative_xml)
    xml_escape("${_name}" _name_xml)
    file(SHA512 "${CPROJECT_STAGING_TEMPLATE_DIR}/${relative}" _hash)
    string(SUBSTRING "${_hash}" 0 4 _guid_part1)
    string(SUBSTRING "${_hash}" 4 4 _guid_part2)
    string(SUBSTRING "${_hash}" 8 4 _guid_part3)
    string(SUBSTRING "${_hash}" 12 12 _guid_part4)
    set(_guid "B1B2B3B4-${_guid_part1}-${_guid_part2}-${_guid_part3}-${_guid_part4}")
    file(APPEND "${_body_file}"
        "${indent}<Component Id=\"Comp_${_identifier}\" Guid=\"${_guid}\">\n"
        "${indent}  <File Id=\"File_${_identifier}\" Name=\"${_name_xml}\" "
        "Source=\"\$(var.CPROJECT.StagingDirTemplate)\\${_windows_relative_xml}\" KeyPath=\"yes\" />\n"
        "${indent}</Component>\n")
    file(APPEND "${_refs_file}" "${indent}  <ComponentRef Id=\"Comp_${_identifier}\" />\n")
endfunction()

function(emit_directory relative indent)
    set(_directory "${CPROJECT_STAGING_TEMPLATE_DIR}")
    if(NOT relative STREQUAL "")
        set(_directory "${_directory}/${relative}")
    endif()
    file(GLOB _entries RELATIVE "${CPROJECT_STAGING_TEMPLATE_DIR}" "${_directory}/*")
    list(SORT _entries)

    set(_files "")
    set(_directories "")
    foreach(_entry IN LISTS _entries)
        if(IS_DIRECTORY "${CPROJECT_STAGING_TEMPLATE_DIR}/${_entry}")
            list(APPEND _directories "${_entry}")
        else()
            list(APPEND _files "${_entry}")
        endif()
    endforeach()

    foreach(_file IN LISTS _files)
        emit_file("${_file}" "${indent}")
    endforeach()

    foreach(_directory_entry IN LISTS _directories)
        get_filename_component(_name "${_directory_entry}" NAME)
        string(REGEX REPLACE "[^A-Za-z0-9_]" "_" _identifier "${_directory_entry}")
        xml_escape("${_name}" _name_xml)
        file(APPEND "${_body_file}" "${indent}<Directory Id=\"Dir_${_identifier}\" Name=\"${_name_xml}\">\n")
        emit_directory("${_directory_entry}" "${indent}  ")
        file(APPEND "${_body_file}" "${indent}</Directory>\n")
    endforeach()
endfunction()

emit_directory("" "      ")
file(READ "${_body_file}" _body)
file(READ "${_refs_file}" _refs)
file(REMOVE "${_body_file}" "${_refs_file}")

file(WRITE "${CPROJECT_OUTPUT}"
"<?xml version=\"1.0\" encoding=\"utf-8\"?>
<!--
  TemplateTree.wxs - GENERATED FILE.
  Lists every file under installer-staging/share/c-project-standard/ while
  preserving its directory hierarchy. Do not edit by hand.
-->
<Wix xmlns=\"http://wixtoolset.org/schemas/v4/wxs\">
  <Fragment>
    <DirectoryRef Id=\"INSTALLDIR_share_cproject\">
${_body}    </DirectoryRef>
    <ComponentGroup Id=\"TemplateTreeComponents\">
${_refs}    </ComponentGroup>
  </Fragment>
</Wix>
")
