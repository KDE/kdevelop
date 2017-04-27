function get_files
{
    echo kdevelop.xml
}

function po_for_file
{
    case "$1" in
       kdevelop.xml)
           echo kdevelop_xml_mimetypes.po
       ;;
    esac
}

function tags_for_file
{
    case "$1" in
       kdevelop.xml)
           echo comment
       ;;
    esac
}
