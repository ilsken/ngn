{
  'variables': {
    'ngn_shared_libuv%': false
  },
  'targets': [
    {
      'target_name': 'ngn',
      'type': 'executable',
      'msvs_guid': '02410d86-8b77-471f-92b9-241d504a678f',
      'dependencies': [
        'deps/uv/uv.gyp:libuv',
      ],
      'defines': [
        'DEFINE_FOO',
        'DEFINE_A_VALUE=value',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        'file1.cc',
        'file2.cc',
      ],
      'conditions': [
        ['OS=="linux"', {
          'defines': [
            'LINUX_DEFINE',
          ],
          'include_dirs': [
            'include/linux',
          ],
        }],
        ['OS=="win"', {
          'defines': [
            'WINDOWS_SPECIFIC_DEFINE',
          ],
        }, { # OS != "win",
          'defines': [
            'NON_WINDOWS_DEFINE',
          ],
        }]
      ],
    },
  ],
  }
