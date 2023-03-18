{
    "targets": [
        {
            "target_name": "udp",
            "sources": ["lib/addons/udp.cc"],
            "cflags": [ "-O2" ],
            "include_dirs": ["<!(node -e \"require('nan')\")"]
        },
        {
            "target_name": "syn",
            "sources": ["lib/addons/syn.cc"],
            "cflags": [ "-O2" ],
            "include_dirs": ["<!(node -e \"require('nan')\")"]
        }
    ]
}