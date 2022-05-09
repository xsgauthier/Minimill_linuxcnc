import re


class GCode2Ngc():
    def __init__(self):
        self.replacements = [
            [r'(G(?:0|1|2|3|28|92).*)(?:E)([-\+]?[\d\.].*)', r'\1A\2'],
            [r'(G)(?:0)(.*F[0-9]+.*)', r'\g<1>1\g<2>'],
            [r'G10', r'G22'],
            [r'G11', r'G23'],
            [r'(M(?:104|106|109|140|141|190|191).*)(?:S)([\d\.].*)', r'\1P\2'],
            [r'.*M(82|105).*', ''],
            [r'M117\s*(.*)\s*', r'(DISPLAY,\1)\n'],
            [r'(G64.*)(?:S)([\d\.].*)', r'\1P\2']
        ]

        self.endCode = 'M2 ; end of program'
        self.endTerm = r'(?:\s*M2.*|\s%.*)'

        self.regMatch = []
        self.endRegex = None

        self.hasProgramEnd = False

    def compile_regex(self):
        for self.regexString, self.replacement in self.replacements:
            regex = re.compile(self.regexString, flags=re.IGNORECASE)
            self.regMatch.append([regex, self.replacement])
        self.endRegex = re.compile(self.endTerm, flags=re.IGNORECASE)

    def do_regex_replacements(self, line):
        for regex, replacement in self.regMatch:
            line = regex.sub(replacement, line)
        return line

    def prepare_processing(self):
        self.hasProgramEnd = False
        self.compile_regex()

    def process_layer(self, data):
        data = self.do_regex_replacements(data)
        if (not self.hasProgramEnd) and self.endRegex.match(data):  # check for end of program
            self.hasProgramEnd = True
        return data

    def finalize_processing(self, data):
        if not self.hasProgramEnd:
            data += self.endCode + '\n'
        return data

    def process(self, data):
        self.prepare_processing()

        data = self.process_layer(data)

        data = self.finalize_processing(data)

        return data
