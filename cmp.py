import sys
import re
import json
import getopt




class ReturnValue:
    def __init__(self, val: str):
        lst = val.strip().split('|')
        self.val = lst[0]
        self.property = lst[1:]

        self.p_func = False
        self.p_instant = False

        for _ in self.property:
            if _ == 'Func':
                self.p_func = True

            if _ == 'INSTANT':
                self.p_instant = True

    def get_val(self):
        return self.val

    def is_func(self):
        return self.p_func

    def is_instant(self):
        return self.p_instant

    def is_fail(self):
        return self.val == 'F'

    def is_positive(self):
        ptn = re.compile("[1-9]+")
        return ptn.match(self.val)

    def is_strict_positive(self):
        return self.is_positive() and not self.is_instant()

    def is_strict_negative(self):
        return self.val.startswith('-') and not self.is_instant()

    def is_negative(self):
        return self.val.startswith('-')

    def is_zero(self):
        return self.val == '0'

    def is_strict_zero(self):
        return self.is_zero() and not self.is_instant()


class Callee:
    def __init__(self, line: str):
        lst = line.strip().split()
        self.name = lst[0]
        self.rvs = []
        for _ in lst[1:]:
            # magic code for return value '0,1'
            if _.startswith('0,1'):
                self.rvs.append(ReturnValue(_.replace('0,1', '0')))
                self.rvs.append(ReturnValue(_.replace('0,1', '1')))
                continue
            self.rvs.append(ReturnValue(_))

    def get_name(self):
        return self.name

    def get_val(self):
        lst = []
        for _ in self.rvs:
            lst.append(_.get_val())
        return list(set(lst))

    def get_num_val(self):
        lst = self.get_val()
        lst = [_ for _ in lst if re.match('-?\d+', _)]
        return lst

    def has_positive(self):
        for _ in self.rvs:
            if _.is_positive():
                return True
        return False

    def has_negative(self):
        for _ in self.rvs:
            if _.is_negative():
                return True
        return False

    def has_strict_negative(self):
        for _ in self.rvs:
            if _.is_strict_negative():
                return True
        return False

    def has_fail(self):
        for _ in self.rvs:
            if _.is_fail():
                return True
        return False

    def has_func(self):
        for _ in self.rvs:
            if _.is_func():
                return True
        return False

    def has_unknown(self):
        return self.has_func() or self.has_fail()


class ReturnCheck:
    def __init__(self, val: str):
        lst = val.strip().split('|')
        self.symbol = lst[0]
        self.value = lst[1]

    def is_ne_zero(self): # NE|0
        return self.symbol == 'NE' and self.value == '0'

    def is_slt_zero(self): # SLT|0
        return self.symbol == 'SLT' and self.value == '0'

    def is_sgt_zero(self): # SGT|0
        return self.symbol == 'SGT' and self.value == '0'

    def __str__(self):
        return self.symbol + '|' + self.value


class Caller:
    def __init__(self, line: str):
        lst = line.strip().split()
        self.caller = lst[0]
        self.callee = lst[1]
        self.rcs = []
        for _ in lst[2:]:
            self.rcs.append(ReturnCheck(_))

    def __str__(self):
        s = self.caller + ' ' + self.callee
        for _ in self.rcs:
            s = s + ' ' + str(_)
        return s

    def is_single(self):
        return len(self.rcs) == 1

    def has_ne_zero(self):
        return any([_.is_ne_zero() for _ in self.rcs])

    def has_slt_zero(self):
        return any([_.is_slt_zero() for _ in self.rcs])

    def has_sgt_zero(self):
        return any([_.is_sgt_zero() for _ in self.rcs])



class Doc:
    """
        mapping_table mainly contains two parts:
            1. mapping table, map words like length to number
            2. macro and enumeration, map the macro-defined functions
            to real functions, map enumeration described in documents
            to actual numbers.
    """

    def __init__(self, callee_name, sent_lst, mapping_table: dict, macro_enum: dict):
        self.callee_name = callee_name
        self.sent_lst = sent_lst
        self.mapping_table = mapping_table
        self.macro_enum = macro_enum
        self.values = set()  # store the reutrn values that documents describe
        self.parse_sents()
        # API described in documents may be macro-defined
        # real_name reveals the real name
        self.real_name = None
        self.set_realname()

    def set_realname(self):
        if self.callee_name in self.macro_enum.keys():
            self.real_name = self.macro_enum[self.callee_name]

    def cleanup(self, sent):
        """ remove the marks within the sentence """
        marks = ['B<([\s\S]+?)>', ]  # add more marks here
        for _m in marks:
            ptn = re.compile(_m)
            f = ptn.findall(sent)
            for _ in f:
                sent = sent.replace(f'B<{_}>', _)
        return sent

    def parse_sents(self):
        """ convert sentences to numbers """
        for _sent in self.sent_lst:
            _sent = self.cleanup(_sent)
            # start mapping

            # step 1. is any value in mapping table
            for _k, _v in self.mapping_table.items():
                if _k in _sent:
                    if type(_v) == list:
                        [self.values.add(_) for _ in _v]
                    else:
                        self.values.add(_v)

            # step 2. is any value in macro_enum
            for _k, _v in self.macro_enum.items():
                if _k in _sent:
                    if type(_v) == list:
                        [self.values.add(_) for _ in _v if re.match('-?\d+', _)]
                    else:
                        if re.match('-?\d+', _v):
                            self.values.add(_v)

            # step 3. is any immediate number
            num = ['-7', '-6', '-5', '-4', '-3', '-2', '-1', '0']
            num += ['1', '2', '3', '4', '5', '6', '7', '8']
            for _n in num:
                num_form = [f' {_n} ', f' {_n}', f' {_n},', f' {_n}.']
                num_form += [f'\{_n} ', f' \{_n}', f'\{_n},', f',{_n}', f'\{_n}.']
                for _ in num_form:
                    if _ in _sent:
                        self.values.add(_n)

    def __str__(self):
        return self.callee_name + ' ' + ' '.join(list(self.values))

    # --------- following methods provide info of return values ---------------

    def has_gt_zero(self):
        return ('+' in self.values) or ('!0' in self.values)

    def has_lt_zero(self):
        return ('-' in self.values) or ('!0' in self.values)

    def has_non_zero(self):
        return '!0' in self.values or \
                ('+' in self.values and '-' in self.values)

    def has_val(self, val):
        return val in self.values

    def get_val(self):
        return self.values

    def get_name(self):
        return self.callee_name

    def get_real_name(self):
        return self.real_name


class CustomDocCheck:
    """
    callee_lst: list of class Callee
    doc_lst: list of class Doc
    """

    def __init__(self, callee_lst, doc_lst):
        self.callee_lst = callee_lst
        self.doc_lst = doc_lst

    def execute(self):
        # first we construct a dict
        callee_dic = dict()
        for _callee in callee_lst:
            lst = _callee.get_num_val()
            callee_dic[_callee.get_name()] = lst

        # start comparison
        for _doc in self.doc_lst:
            _doc_val = _doc.get_val()
            if len(_doc_val) == 0:
                continue
            _doc_name = _doc.get_name()
            if not _doc_name in callee_dic.keys():
                _doc_name = _doc.get_real_name()
            if not _doc_name in callee_dic.keys():
                continue

            _callee_val = callee_dic[_doc_name]
            flag = False
            for _ in _callee_val:
                if _ in _doc_val:
                    continue
                # negative value
                if _.startswith('-'):
                    if _doc.has_lt_zero():
                        continue
                elif re.match('\d+', _):
                    if _doc.has_gt_zero():
                        continue
                flag = True
            if flag:
                print(_doc.get_name(), '!',' '.join(_doc.sent_lst).replace('\n', ' '), _doc_val, _callee_val)


class CustomCheck:
    """
    callee_lst: list of class Callee
    caller_lst: list of class Caller
    """
    def __init__(self, callee_lst: list, caller_lst: list):
        self.caller_lst = caller_lst
        # construct a dict
        self.callee_dic = dict()
        for _ in callee_lst:
            assert(isinstance(_, Callee))
            self.callee_dic[_.name] = _
        # check type
        self.check_type = []

    def confuse_pos_and_neg(self, caller_ins, strict = False):
        assert(isinstance(caller_ins, Caller))
        if not caller_ins.callee in self.callee_dic.keys():
            return False
        callee = self.callee_dic[caller_ins.callee]
        assert(isinstance(callee, Callee))
        callee_name = callee.get_name()
        # compare-related functions use pos and neg to distinguish relationship
        # instead of correct and error
        if any([_ in callee_name for _ in ['compare', 'cmp']]):
            return False

        bool1 = callee.has_positive() and callee.has_negative()
        if strict:
            bool1 = callee.has_positive() and callee.has_strict_negative()
        bool2 = caller_ins.is_single() and caller_ins.has_ne_zero()

        return bool1 and bool2

    def confuse_pos_and_neg_ssl(self, caller_ins, strict = False):
        assert(isinstance(caller_ins, Caller))
        if not caller_ins.callee in self.callee_dic.keys():
            return False
        callee = self.callee_dic[caller_ins.callee]
        assert(isinstance(callee, Callee))
        callee_name = callee.get_name()
        # compare-related functions use pos and neg to distinguish relationship
        # instead of correct and error
        if any([_ in callee_name for _ in ['compare', 'cmp']]):
            return False

        bool1 = (callee.has_unknown() or callee.has_positive()) and callee.has_negative()
        if strict:
            bool1 = (callee.has_fail() or callee.has_positive()) and callee.has_strict_negative()
        bool2 = caller_ins.is_single() and caller_ins.has_ne_zero()
        return bool1 and bool2

    def nonexist_pos(self, caller_ins, strict = False):
        assert(isinstance(caller_ins, Caller))
        if not caller_ins.callee in self.callee_dic.keys():
            return False
        callee = self.callee_dic[caller_ins.callee]
        if callee.has_unknown():
            return False

        bool1 = not callee.has_positive()
        bool2 = caller_ins.is_single() and caller_ins.has_sgt_zero()

        return bool1 and bool2

    def nonexist_neg(self, caller_ins, strict = False):
        assert(isinstance(caller_ins, Caller))
        if not caller_ins.callee in self.callee_dic.keys():
            return False
        callee = self.callee_dic[caller_ins.callee]
        if callee.has_unknown():
            return False

        bool1 = not callee.has_negative()
        bool2 = caller_ins.is_single() and caller_ins.has_slt_zero()

        return bool1 and bool2

    def set_type(self, func, is_strict = False):
        self.check_type.append([func, is_strict])

    def execute(self):
        for _caller in self.caller_lst:
            if any([_[0](*([_caller, _[1]])) for _ in self.check_type]):
                print(_caller)


def load_file(return_value, cops, doc, mapping = None):
    # read files and generate list of Callee and Caller
    callee_lst = []
    with open(return_value, 'r') as r:
        for _line in r.readlines():
            # there is return value like '0,1'
            callee_lst.append(Callee(_line.strip()))

    caller_lst = []
    with open(cops, 'r') as r:
        for _line in r.readlines():
            caller_lst.append(Caller(_line.strip()))

    # load mapping first
    map_table = {
        'positive': '+',
        'negative': '-',
        'Negative': '-',
        'non-negative': ['+', '0'],
        ' zero': '0',
        'zero': '0',
        'nonzero': '!0',
        'non zero': '!0',
        'non-zero': '!0',
        'Non-zero': '!0',
        'non-0': '!0',
        '!0': '!0',
        'number': '+',
        'amount': '+',
        'length': '+',
        'Length': '+',
        'index': '+',
        'offset': '+',
        'depth': '+',
        'count': '+',
        'position': '+',
        'size': '+',
        '<0': '-',
        '< 0': '-',
        '<=0': '-',
        'less than or equal to 0': ['-', '0'],
        '!=0': '!0',
        '!= 0': '!0',
        'error code': ['+', '-'],
        'error-code': ['+', '-'],
        'error number': ['+', '-'],
        'error value': ['+', '-'],
        # new for gnutls
        '%GNUTLS_E_SUCCESS': '0',
        '0 ': '0',
        'Zero': '0',
        '(0)': '0',
        '(-1)': '-1',
        ' 1 ': '1',
        'TRUE': '1',
        'FALSE': '0',
        'true': '1',
        'True': '1',
        'False': '0',
        'false': '0',
    }

    # macro and enumeration
    macro_enum = dict()
    if mapping: # file mapping stores macro and enumeration info
        with open(mapping, 'r') as r:
            for _line in r.readlines():
                _line = _line.strip().split()
                macro_enum[_line[0]] = _line[1]
    # print(macro_enum)

    doc_lst = list()
    doc_dict = None
    with open(doc, 'r') as r:
        doc_dict = json.loads(r.read())
    assert(doc_dict)

    for _k, _v in doc_dict.items():
        if type(_v) == str:
            _v = [_v]
        doc_lst.append(Doc(_k, _v, map_table, macro_enum))

    return (callee_lst, caller_lst, doc_lst)


if __name__ == '__main__':
    argv = sys.argv[1:]
    opts, args = getopt.getopt(argv, 's', ["cops=", "cbp=", "doc=", "mapping=", "nonexist_neg", "nonexist_pos", "confuse_pos_and_neg", "confuse_pos_and_neg_ssl"])

    cops = cbp = doc = mapping = ''
    nonexist_neg = nonexist_pos = confuse_pos_and_neg = confuse_pos_and_neg_ssl = False
    strict = False

    for opt, arg in opts:
        if opt == '--cops':
            cops = arg
        elif opt == '--cbp':
            cbp = arg
        elif opt == '--doc':
            doc = arg
        elif opt == '--mapping':
            mapping = arg
        elif opt == '--nonexist_neg':
            nonexist_neg = True
        elif opt == '--nonexist_pos':
            nonexist_pos = True
        elif opt == '--confuse_pos_and_neg':
            confuse_pos_and_neg = True
        elif opt == '--confuse_pos_and_neg_ssl':
            confuse_pos_and_neg_ssl = True
        elif opt == '-s':
            strict = True
        else:
            print("invalid parameter: " + opt)
            exit()

    if not (cops and cbp):
        print("Assign cops and cbp results!\n")
        exit()

    callee_lst, caller_lst, doc_lst = load_file(cbp, cops, doc, mapping)

    # callee check
    print('-------------------------- caller check --------------------------\n')
    ccheck = CustomCheck(callee_lst, caller_lst)

    if nonexist_neg:
        ccheck.set_type(ccheck.nonexist_neg, strict)
    if nonexist_pos:
        ccheck.set_type(ccheck.nonexist_pos, strict)
    if confuse_pos_and_neg:
        ccheck.set_type(ccheck.confuse_pos_and_neg, strict)
    if confuse_pos_and_neg_ssl:
        ccheck.set_type(ccheck.confuse_pos_and_neg_ssl, strict)

    ccheck.execute()

    # doc check
    print('-------------------------- doc check --------------------------\n')
    dcheck = CustomDocCheck(callee_lst, doc_lst)
    dcheck.execute()
