library %{APPNAMELC};

function %{APPNAMELC}_test: integer; cdecl; export;
begin
    %{APPNAMELC}_test := 0;
end;

exports
    %{APPNAMELC}_test;

end.
