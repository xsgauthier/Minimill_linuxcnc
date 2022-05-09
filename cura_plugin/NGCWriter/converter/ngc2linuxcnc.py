import sys
import re
import math


class Ngc2LinCNC():
    def ___init__(self):
        pass

    def init_variables(self):
        self.regMatch = {}
        self.line_count = 0
        self.output_line_count = 0
        self.currentInFile = None
        self.currentOutFile = None
        self.retracted = True
        self.outputData = []

        self.relative_extrusion = False
        self.current_a = 0.0
        self.prev_p = [None, None, None, None, None]

    def getCodeInt(self, line, code):
        if code not in self.regMatch:
            self.regMatch[code] = re.compile(code + r'([^\s]+)', flags=re.IGNORECASE)
        m = self.regMatch[code].search(line)
        if m is None:
            return None
        try:
            return int(m.group(1))
        except ValueError:
            return None

    def getCodeFloat(self, line, code):
        if code not in self.regMatch:
            self.regMatch[code] = re.compile(code + r'([^\s]+)', flags=re.IGNORECASE)
        m = self.regMatch[code].search(line)
        if m is None:
            return None
        try:
            return float(m.group(1))
        except ValueError:
            return None

    def OutG(self, g, p, c):
        self.output_line_count = self.output_line_count + 1
        s = "G" + str(g) + " "
        if (p[0] is not None) and (p[0] != self.prev_p[0]):
            self.prev_p[0] = p[0]
            s = s + "X{0:g}".format(p[0]) + " "
        if (p[1] is not None) and (p[1] != self.prev_p[1]):
            self.prev_p[1] = p[1]
            s = s + "Y{0:g}".format(p[1]) + " "
        if (p[2] is not None) and (p[2] != self.prev_p[2]):
            self.prev_p[2] = p[2]
            s = s + "Z{0:g}".format(p[2]) + " "
        if (p[3] is not None) and (p[3] != self.prev_p[3]):
            self.prev_p[3] = p[3]
            s = s + "A{0:g}".format(p[3]) + " "
        if (p[4] is not None) and (p[4] != self.prev_p[4]):
            self.prev_p[4] = p[4]
            s = s + "F{0:g}".format(p[4]) + " "
        if c is not None:
            s = s + "; " + c
        s = s.rstrip()
        self.outputLine(s)


    def compareValue(self, newValue, oldValue, tolerance):
        return (newValue < (oldValue * (1.0 - tolerance))) \
            or (newValue > (oldValue * (1.0 + tolerance)))

    def outputLine(self, line):
        self.outputData.append(line + '\n')

    def prepare_processing(self):
        self.init_variables()

    def process_line(self, line):
        self.line_count = self.line_count + 1
        line = line.rstrip()
        original_line = line
        if type(line) is tuple:
            line = line[0]

        # Extract comment
        if ';' in line or '(' in line:
            sem_pos = line.find(';')
            par_pos = line.find('(')
            pos = sem_pos
            if pos is None:
                pos = par_pos
            elif par_pos is not None:
                if par_pos > sem_pos:
                    pos = par_pos
            comment = line[pos + 1:].strip()
            line = line[0:pos]
        else:
            comment = None

        M = self.getCodeInt(line, 'M')
        G = self.getCodeInt(line, 'G')
        if M == 83:
            self.relative_extrusion = True
        elif G == 1:    # Move
            x = self.getCodeFloat(line, 'X')
            y = self.getCodeFloat(line, 'Y')
            z = self.getCodeFloat(line, 'Z')
            a = self.getCodeFloat(line, 'A')
            f = self.getCodeFloat(line, 'F')

            # print(f"; G{G} A{a} RX{self.relative_extrusion}")
            output_g92 = False
            if a is not None:
                if a < 0.0:
                    output_g92 = True

                if self.relative_extrusion:
                    self.current_a += a
                else:
                    self.current_a = a

            if output_g92:
                self.OutG(92, [x, y, z, self.current_a, None], "Reset Axis position")
            else:
                self.OutG(G, [x, y, z, self.current_a, f], comment)
        else:
            self.outputLine(original_line)
            self.output_line_count = self.output_line_count + 1

    def finalize_processing(self, data):
        self.outputLine("; GCode file processed by ngc2LinCNC")
        self.outputLine("; Input Line Count = " + str(self.line_count))
        self.outputLine("; Output Line Count = " + str(self.output_line_count))

        data += ''.join(self.outputData)
        #self.optimizeCross()
        return data

    def process(self, data):
        self.prepare_processing()

        for line in data.split('\n'):
            if line == '':
                continue
            self.process_line(line)
        data = ''.join(self.outputData)
        self.outputData = []

        data = self.finalize_processing(data)
        return data

if __name__=="__main__":
    c = Ngc2LinCNC()
    with open(sys.argv[1], "r") as f:
        print(c.process('\n'.join(f.readlines())))