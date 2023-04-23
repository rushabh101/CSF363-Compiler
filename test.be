fun moo(b : short) : int {
    let abc : int = b;
    dbg abc;

    if 1 {
        ret 1;
    }
    else {
        ret 2;
    }
}

fun main() : int {
    let abc : int = 20000;

    dbg moo(69);


    if 0 {
        let a : short = 1;
        dbg a;
    } else {
        let a : short = 2;
        dbg a;
    }
}



