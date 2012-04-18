{
  "targets": [
    {
      "target_name": "nroonga_bindings",
      "sources": [ "src/nroonga.cc" ],
      "include_dirs": ["<!@(pkg-config --cflags-only-I groonga | sed -e 's/-I//g')"],
      "libraries": ["<!@(pkg-config --libs-only-l groonga)"]
    }
  ]
}
