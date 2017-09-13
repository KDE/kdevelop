function get_files
{
    echo kdevclang.xml
}

function po_for_file
{
    case "$1" in
       kdevclang.xml)
           echo kdevclang_xml_mimetypes.po
       ;;
    esac
}

function tags_for_file
{
    case "$1" in
      kdevclang.xml)
           echo comment
       ;;
    esac
}
