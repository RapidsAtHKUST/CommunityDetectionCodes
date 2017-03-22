import re


def gml_formatter(in_file, out_file):
    with open(in_file) as ifs:
        lines = filter(lambda line: 'Creator' not in line, ifs.readlines())
        whole_content = ''.join(lines)
        new_content = re.sub('\s+\[', ' [', whole_content)
        print new_content
        with open(out_file, 'w') as ofs:
            ofs.write(new_content)


if __name__ == '__main__':
    gml_formatter('zachary_karate_club/original/karate.gml', 'zachary_karate_club/original/karate_new.gml')
