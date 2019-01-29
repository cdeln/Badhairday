import sys
import re

if len(sys.argv) != 3:
    raise Exception('needs input and output model')

input_model_path = sys.argv[1]
output_model_path = sys.argv[2]

output_model = ''

def permute(s):
    a, b, c = s.split('/')
    return '/'.join([a,c,b])

with open(input_model_path, 'r') as f:
    for line in f.readlines():
        tokens = re.split('[ \n]+', line)
        tokens = [t for t in tokens if t]
        if len(tokens) == 4:
            typ, a, b, c = tokens
            if typ == 'vn':
                a = str(-float(a))
                b = str(-float(b))
                c = str(-float(c))
                tokens = [typ,a,b,c]
            #elif typ == 'f':
            #    tokens = [typ,a,c,b]
        output_model += ' '.join(tokens) + '\n'

with open(output_model_path, 'w') as f:
    f.write(output_model)
