// Examples taken from the Java specification.
// First the types are defined, further down they are used.

@Info(value = "javaparser/annotationtest")
package org.kde.kdevelop.javaparser;


//
// Annotation types
//

// Normal annotation type declaration with several elements

// Describes the "request-for-enhancement" (RFE) that led
// to the presence of the annotated API element.
public @interface RequestForEnhancement {
     int    id();       // Unique ID number associated with RFE
     String synopsis(); // Synopsis of RFE
     String engineer(); // Name of engineer who implemented RFE
     String date();     // Date RFE was implemented
}


// Marker annotation type declaration

// Annotation with this type indicates that the specification of the
// annotated API element is preliminary and subject to change.
public @interface Preliminary { }


// Single-element annotation type declaration with array-typed element

// Associates a list of endorsers with the annotated class.
public @interface Endorsers {
     String[] value();
}


// Complex Annotation Type

// A person’s name. This annotation type is not designed to be used
// directly to annotate program elements, but to define elements
// of other annotation types.
public @interface Name {
     String first();
     String last();
}

// Indicates the author of the annotated program element.
public @interface Author {
     Name value();
}


//
// Annotation uses
//


// Marker annotation
@Preliminary public class TimeTravel
{
	// Normal annotation
	@RequestForEnhancement(
		id       = 2868724,
		synopsis = "Provide time-travel functionality",
		engineer = "Mr. Peabody",
		date     = "4/1/2004"
	)
	public static void travelThroughTime(Date destination)
	{ }
}

// Array-valued single-element annotation
@Endorsers({"Children", "Unscrupulous dentists"})
public class Lollipop { }

// Single-element array-valued single-element annotation
@Endorsers("Epicurus")
public class Pleasure { }

// Single-element complex annotation
@Author(@Name(first = "Joe", last = "Hacker"))
public class BitTwiddle { }
