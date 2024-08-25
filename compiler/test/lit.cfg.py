import lit.formats

config.name = 'Parser Test'
config.test_format = lit.formats.ShTest(True)
config.suffixes = ['.rx']
config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = config.test_source_root

config.substitutions.append(('%parse-tree', os.environ.get('PARSE_TREE_BIN', '')))
config.substitutions.append(('%rx-frontend', os.environ.get('FRONT_END_BIN', '')))

