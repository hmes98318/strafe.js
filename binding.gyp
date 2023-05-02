{
    "targets": [
        {
            "target_name": "raw-udp",
            "sources": ["lib/addons/raw_udp.cc"],
            "cflags": [ "-O2" ],
            "include_dirs": ["<!(node -e \"require('nan')\")"]
        },
        {
            "target_name": "raw-syn",
            "sources": ["lib/addons/raw_syn.cc"],
            "cflags": [ "-O2" ],
            "include_dirs": ["<!(node -e \"require('nan')\")"]
        }
    ]
}