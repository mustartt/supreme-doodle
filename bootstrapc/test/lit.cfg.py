import lit.formats

config.name = 'Parser Test'
config.test_format = lit.formats.ShTest(True)
config.suffixes = ['.lang']
config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = config.test_source_root
