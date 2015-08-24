import ".././files/directory_to_import/" as MyDir
import "types.js" as MyTypes
import org.kde.kdevplatform as KDevPlatform

/**
 * "internalContext" : { "importedParents" : { "0" : {"type" : "Class" }}}
 */
MyDir.MyComponent {
    id: component

    onTest: {
        /**
         * "type" : { "toString" : "bool" }
         */
        var from_types_js = MyTypes.simple_compare;

        /**
         * "EXPECT_FAIL" : {"type" : { "toString": "ICore" }}
         */
        var icore = KDevPlatform.ICore;
    }
}
