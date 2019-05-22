{
  "targets": [
    {
      "target_name": "nroonga_bindings",
      "sources": [ "src/nroonga.cc" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ],
      "conditions": [
        ['OS == "mac"', {
          "xcode_settings": {
            "OTHER_LDFLAGS": ["<!@(pkg-config --libs-only-L groonga)"]
          },
          "include_dirs": [
            "<!@(pkg-config --cflags-only-I groonga | sed -e 's/-I//g')",
          ],
          "ldflags": ["<!@(pkg-config --libs-only-L groonga)"],
          "libraries": ["<!@(pkg-config --libs-only-l groonga)"],
        }],
        ['OS=="win"', {
          "include_dirs": [
            "<!(echo %GROONGA_PATH%)/include",
            "<!(echo %GROONGA_PATH%)/include/groonga",
          ],
          "library_dirs": ["<!(echo %GROONGA_PATH%)/lib"],
          "libraries": ["libgroonga"],
        }, {
          "include_dirs": [
            "<!@(pkg-config --cflags-only-I groonga | sed -e 's/-I//g')",
          ],
          "ldflags": ["<!@(pkg-config --libs-only-L groonga)"],
          "libraries": ["<!@(pkg-config --libs-only-l groonga)"],
        }]
      ]
    }
  ]
}
