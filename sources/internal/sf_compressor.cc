// Copyright 2021-2023 PANDA Gmbh

#include "internal/sf_compressor.h"

#include <array>
#include <cmath>
#include <iostream>

namespace drift::wavelet::internal {

const int MAXCODELEN = 32;
/**
 * Original struct. Don't change
 */
typedef struct {
  uint8_t Exp;       // Exponent of the bfloat 16 (real Exp=Exp-127)
  uint8_t Sgn;       // Sgn (1=Minus)
  uint32_t SgnFrag;  // Sgn & Fragment of the bfloat 16 (MSB=1:Minus)
} bfloat16;

typedef struct {
  uint8_t BitsUsed;  // gültige LSB-Bits
  uint32_t BitsOut;  // Ausgabewort
} typeOutAdder;

struct typePool {
  // Poolcounter(Max possible/Used)
  typePool() = default;

  uint8_t PoolsCount, PoolsUsed;
  // Steps for optimal digital Search
  uint8_t BisecSteps;
  std::array<uint8_t, 6> BisecStep;  // 2^6 systematic for 32-Bit(2x32)
  // Borders of Poolvalues
  std::array<uint32_t, 2 * MAXCODELEN> Last, CodeQty;
  // Dataset Counter for each Pool
  std::array<uint32_t, 2 * MAXCODELEN> DSQty, DSQtyUp;
  // CodeOffset for Codeout
  std::array<uint32_t, 2 * MAXCODELEN> CodeOffset;
  // Codelen for each Pool
  std::array<uint8_t, 2 * MAXCODELEN> CodeLen;
  // PoolAmount for Output
  std::array<uint8_t, 2 * MAXCODELEN> LastPoolNr;
};

class SfCompressor::Impl {
 public:
  explicit Impl(size_t buffer_size)
      : OutData(buffer_size),
        ZeroCount(buffer_size),
        Fragment(buffer_size),
        ExpJump(buffer_size) {
    uint32_t t;
    uint8_t PNr;
    /**
     * Original code snippet. Don't change it.
     */
    // Calculate Pow(2) as Shortcuts
    Pow2Int32[0] = 1;
    for (t = 1; t < MAXCODELEN; t++) {
      Pow2Int32[t] = 2 * Pow2Int32[t - 1];
    }
    Pow2Dbl129[0] = 1.0;
    for (t = 1; t <= 128; t++) {
      Pow2Dbl129[t] = 2.0 * Pow2Dbl129[t - 1];
    }

    // Setze Codeborders for Exp-Jumps und Zeros
    for (PNr = 0; PNr < 2; PNr++) {
      Pool[PNr].PoolsCount =
          (PNr == 0 ? 16 : 63);  // 16=(2*8+1)-{1 wg. Speed},63=(2*31+1)
      for (t = 0; t < 5; t++) {
        Pool[PNr].Last[t] = ((PNr == 0 && t > 2) ? 2 * (t - 1) : t);
      }
      for (t = 5; t < Pool[PNr].PoolsCount; t++) {
        Pool[PNr].Last[t] = 2 * Pool[PNr].Last[t - 2];
      }
      Pool[PNr].CodeQty[0] = 1;
      for (t = 1; t < Pool[PNr].PoolsCount; t++) {
        Pool[PNr].CodeQty[t] = Pool[PNr].Last[t] - Pool[PNr].Last[t - 1];
      }
      // Erzeuge Suchmaske für Eingabepools
      setPoolSearch(PNr);
    }
    // Suchmaske für Shortcut Log2
    for (t = 0; t < 32; t++) Pool[4].Last[t] = Pow2Int32[t];
    Pool[4].PoolsCount = 32;
    setPoolSearch(4);
    for (t = 0; t < 31; t++) Pool[5].Last[t] = Pool[4].Last[t + 1] - 1;
    Pool[5].Last[31] = 0xFFFFFFFF;
    Pool[5].PoolsCount = 32;
    setPoolSearch(5);

    /**
     * End of original code snippet. Don't change it.
     */
  }

  int Compress(const SfCompressor::OriginalData &origin,
               std::vector<uint8_t> *blob) {
    if (origin.frag_length < 7 || origin.frag_length > 23 ||
        origin.frag_length == 22) {
      return 0;
    }

    if (origin.values.size() > origin.columns * origin.rows) {
      return 0;
    }

    if (origin.values.size() != origin.indexes.size()) {
      return 0;
    }

    uint32_t t = 0;
    uint8_t PNr = 0;
    double FreeCodes;
    uint8_t SgnUsed;
    int LastExpTmp;
    int JumpSize, MaxJumpSize;
    uint8_t PoolsReady, NextPoolsReady;  // Abgearbeitete Pools => Codelängen
    uint32_t CodesReady;                 // Letzter abgearbeiteter Eintrag
    uint32_t CountUpsReady;              // Abgearbeitete Vorkommensumme
    double Sollanteil, Istanteil = 0;
    uint8_t PZiel;
    bool M_RowBased = origin.row_based;

    uint32_t M_NonZeroSize = origin.indexes.size();
    uint32_t TimelineLen;

    auto M_Index = origin.indexes.data();
    auto M_Value = origin.values.data();
    bfloat16 M_Value16;

    OutBitsFree = 0;
    OutBytes = 0;
    uint32_t M_Row = origin.rows;
    uint32_t M_Col = origin.columns;
    FragLen = origin.frag_length;

    /**
     * Original code snippet. Don't change it. Quelletext.cpp:117
     */
    TimelineLen = 0;
    LastExpTmp = 127;
    SgnUsed = 0;
    for (PNr = 0; PNr < 2; PNr++) {
      for (t = 0; t < Pool[PNr].PoolsCount; t++) {
        Pool[PNr].DSQty[t] = 0;
        Pool[PNr].DSQtyUp[t] = 0;
      }
    }
    for (t = 0; t < M_NonZeroSize; t++) {
      ZeroCount[TimelineLen] =
          M_Index[t] - ((t > 0) ? (1 + M_Index[t - 1]) : 0);
      // Convert Floatvalue to b_float
      M_Value16 = float_to_bfloat16(M_Value[t]);
      // Show_bfloat16(M_Value16);
      SgnUsed = (M_Value16.Sgn + 1) | SgnUsed;
      if (M_Value16.Exp >= LastExpTmp) {
        JumpSize = M_Value16.Exp - LastExpTmp;
        // Bidir Jump possible up to LastExp
        ExpJump[TimelineLen] =
            JumpSize + ((LastExpTmp < JumpSize) ? LastExpTmp : JumpSize);
      } else {
        JumpSize = LastExpTmp - M_Value16.Exp;
        MaxJumpSize = 255 - LastExpTmp;
        // Bidir Jump possible up to (255-LastExp)
        ExpJump[TimelineLen] =
            JumpSize + ((MaxJumpSize < JumpSize) ? MaxJumpSize : JumpSize - 1);
      }
      // Data to Poolcounter
      Pool[0].DSQty[findPoolNr(0, ExpJump[TimelineLen])]++;
      Pool[1].DSQty[findPoolNr(1, ZeroCount[TimelineLen])]++;
      Fragment[TimelineLen] = M_Value16.SgnFrag;
      // Reset by Iterator
      TimelineLen++;
      // Copy for next Jump-Calculations
      LastExpTmp = M_Value16.Exp;
    }
    // Ergänze Nullen am Ende (auch wenn n.v., damit Dateiende definiert...)
    // JumpSize=255-LastExpTmp;
    // ExpJump[TimelineLen]=JumpSize+((LastExpTmp<JumpSize)?LastExpTmp:JumpSize);
    // ZeroCount[TimelineLen]=(M_NonZeroSize>0)?M_Row*M_Col-(M_Index[M_NonZeroSize-1]+1):M_Row*M_Col;
    // Pool[0].DSQty[findPoolNr(0,ExpJump[TimelineLen])]++;
    // Pool[1].DSQty[findPoolNr(1,ZeroCount[TimelineLen])]++;
    // TimelineLen++;
    // ###################################################################
    // Look for last used Container & calculate Summs of Datasets for both Codes
    for (PNr = 0; PNr < 2; PNr++) {
      Pool[PNr].PoolsUsed = 0;
      for (t = Pool[PNr].PoolsCount; t > 0; t--) {
        if (Pool[PNr].DSQty[t - 1] > 0) {
          Pool[PNr].PoolsUsed = t;
          break;
        }
      }
      Pool[PNr].DSQtyUp[0] = Pool[PNr].DSQty[0];
      for (t = 1; t < Pool[PNr].PoolsUsed; t++) {
        Pool[PNr].DSQtyUp[t] = Pool[PNr].DSQtyUp[t - 1] + Pool[PNr].DSQty[t];
      }
      // for(t=0;t<Pool[PNr].PoolsUsed;t++)cout<<t<<" "<<Pool[PNr].DSQty[t]<<"
      // "<<Pool[PNr].DSQtyUp[t]<<" "<<Pool[PNr].Last[t]<<"\n";
    }
    // ###################################################################
    //  Erstelle Codezuordnungen mit abgerundeter Menge
    for (PNr = 0; PNr < 2; PNr++) {
      FreeCodes =
          Pow2Dbl129[32] - (Pool[PNr].Last[Pool[PNr].PoolsUsed - 1] + 1);
      PoolsReady = 0;
      CodesReady = 0;
      CountUpsReady = 0;
      for (uint32_t TestCodeLen = 0; TestCodeLen <= MAXCODELEN; TestCodeLen++) {
        NextPoolsReady = PoolsReady;
        for (t = PoolsReady; t < Pool[PNr].PoolsUsed; t++) {
          Sollanteil = static_cast<double>(Pool[PNr].Last[t] + 1 - CodesReady) *
                       (Pow2Dbl129[32 - TestCodeLen] - 1.0) / FreeCodes;
          if (Sollanteil > 1) {
            break;
          }  // Mit dieser CodeLen nicht mehr möglich
          Istanteil =
              static_cast<double>(Pool[PNr].DSQtyUp[t] - CountUpsReady) /
              static_cast<double>(TimelineLen);
          if (Istanteil >= Sollanteil) {
            NextPoolsReady = t + 1;
          }
          // cout<<"t: "<<t<<" TestCodeLen: "<<TestCodeLen<<" Sollanteil
          // "<<Sollanteil<<" Istanteil "<<Istanteil<<"\n";
        }
        if (NextPoolsReady > PoolsReady) {
          for (t = PoolsReady; t < NextPoolsReady; t++) {
            Pool[PNr].CodeLen[t] = TestCodeLen;
          }
          CodesReady = Pool[PNr].Last[NextPoolsReady - 1] + 1;
          FreeCodes =
              Pow2Dbl129[32] -
              (Pool[PNr].Last[Pool[PNr].PoolsUsed - 1] + 1 - CodesReady);
          CountUpsReady = Pool[PNr].DSQtyUp[NextPoolsReady - 1];
          PoolsReady = NextPoolsReady;
        }
        if (PoolsReady == Pool[PNr].PoolsUsed) {
          break;
        }
      }
      // ###################################################################
      //  Verteile den Rest "von oben nach unten"
      FreeCodes = 1;  // Setze Bereich auf 100% frei
      for (t = 0; t < Pool[PNr].PoolsUsed; t++) {
        FreeCodes -= static_cast<double>(Pool[PNr].CodeQty[t]) /
                     Pow2Dbl129[Pool[PNr].CodeLen[t]];
      }
      while (FreeCodes > 0 && Pool[PNr].PoolsUsed > 0) {
        for (t = 0; t < Pool[PNr].PoolsUsed; t++) {
          // cout<<t<<": "<<1*Pool[PNr].CodeLen[t]<<"=>";
          // Pruefen, ob Codeanteil vergroessert (2x) werden kann
          if (FreeCodes >= static_cast<double>(Pool[PNr].CodeQty[t]) /
                               Pow2Dbl129[Pool[PNr].CodeLen[t]]) {
            FreeCodes -= static_cast<double>(Pool[PNr].CodeQty[t]) /
                         Pow2Dbl129[Pool[PNr].CodeLen[t]];
            Pool[PNr].CodeLen[t]--;
            // cout<<1*Pool[PNr].CodeLen[t];
            if (FreeCodes == 0) {
              break;
            }
          }
          // cout<<"\n";
        }
      }
      // cout<<"\n"<<"Pools Used: "<<1*Pool[PNr].PoolsUsed<<"\n";
    }
    // ###################################################################
    //  Erzeuge Ausgabepools
    for (PNr = 0; PNr < 2; PNr++) {
      PZiel = PNr + 2;
      Pool[PZiel].PoolsCount = 1;
      Pool[PZiel].LastPoolNr[0] = 0;
      Pool[PZiel].CodeLen[0] = Pool[PNr].CodeLen[0];
      Pool[PZiel].CodeOffset[0] = 0;
      for (t = 1; t < Pool[PNr].PoolsUsed; t++) {
        if (Pool[PNr].CodeLen[t] > Pool[PNr].CodeLen[t - 1]) {
          Pool[PZiel].Last[Pool[PZiel].PoolsCount - 1] = Pool[PNr].Last[t - 1];
          Pool[PZiel].CodeLen[Pool[PZiel].PoolsCount] = Pool[PNr].CodeLen[t];
          if (Pool[PZiel].PoolsCount > 1) {
            // Nächsten Code letzte CodeLen
            Pool[PZiel].CodeOffset[Pool[PZiel].PoolsCount - 1] =
                (Pool[PZiel].CodeOffset[Pool[PZiel].PoolsCount - 2] +
                 Pool[PZiel].Last[Pool[PZiel].PoolsCount - 2] + 1);
            // Schieben auf neue CodeLen
            Pool[PZiel].CodeOffset[Pool[PZiel].PoolsCount - 1] =
                (Pool[PZiel].CodeOffset[Pool[PZiel].PoolsCount - 1]
                 << (Pool[PZiel].CodeLen[Pool[PZiel].PoolsCount - 1] -
                     Pool[PZiel].CodeLen[Pool[PZiel].PoolsCount - 2]));
            // Neuen Startwert abziehen
            Pool[PZiel].CodeOffset[Pool[PZiel].PoolsCount - 1] -=
                (Pool[PZiel].Last[Pool[PZiel].PoolsCount - 2] + 1);
          }
          Pool[PZiel].PoolsCount++;
        }
        Pool[PZiel].LastPoolNr[Pool[PZiel].PoolsCount - 1] = t;
      }
      Pool[PZiel].Last[Pool[PZiel].PoolsCount - 1] =
          Pool[PNr].Last[Pool[PNr].PoolsUsed - 1];
      if (Pool[PZiel].PoolsCount > 1) {
        // Nächsten Code letzte CodeLen
        Pool[PZiel].CodeOffset[Pool[PZiel].PoolsCount - 1] =
            (Pool[PZiel].CodeOffset[Pool[PZiel].PoolsCount - 2] +
             Pool[PZiel].Last[Pool[PZiel].PoolsCount - 2] + 1);
        // Schieben auf neue CodeLen
        Pool[PZiel].CodeOffset[Pool[PZiel].PoolsCount - 1] =
            (Pool[PZiel].CodeOffset[Pool[PZiel].PoolsCount - 1]
             << (Pool[PZiel].CodeLen[Pool[PZiel].PoolsCount - 1] -
                 Pool[PZiel].CodeLen[Pool[PZiel].PoolsCount - 2]));
        // Neuen Startwert abziehen
        Pool[PZiel].CodeOffset[Pool[PZiel].PoolsCount - 1] -=
            (Pool[PZiel].Last[Pool[PZiel].PoolsCount - 2] + 1);
      }
      Pool[PZiel].PoolsUsed = Pool[PZiel].PoolsCount;
      // Erzeuge Suchmaske für Ausgabepools
      setPoolSearch(PZiel);
    }
    for (PNr = 0; PNr < 6; PNr++) ShowPoolSearch(PNr, true);
    // ###################################################################
    if (static_cast<double>(M_Row) * static_cast<double>(M_Col) >
        Pow2Dbl129[31]) {
      std::cerr << "Matrix zu gross, keine Ausgabe";
      return 0;
    }
    // ###################################################################
    int LastDefinedCode;
    int LastDefinedPool;
    int LastDefinedCodeLen;
    uint32_t MaxPossCode;
    uint8_t MaxPossPool;
    uint8_t RowColCodeLen;
    uint32_t FragCodes;
    bool ZerosAppear;
    // typeOutAdder AusgabeCode;

    OutBitsFree = 0;
    OutBytes = 0;

    //    cout << "Header\n\n";
    // VersionsNr 0-2: 2Bit, 3: weitere Bits
    Bitstream(SevenOfNine(0, 4));
    //    cout << ",VersionsNr=0\n";
    ShowBitstream();
    // 1 Bit für Row-Based
    Bitstream(SevenOfNine(M_RowBased, 2));
    //    cout << ",Row-Based=" << 1 * M_RowBased << "\n";
    ShowBitstream();
    // Kann Werte annehmen: 0..2^31-1
    RowColCodeLen = findPoolNr(4, ((M_Row > M_Col) ? M_Row : M_Col) + 1);
    Bitstream(SevenOfNine(RowColCodeLen, 32));
    //    cout << ",RowColCodeLen=" << 1 * RowColCodeLen << "\n";
    ShowBitstream();
    // RowColCodeLen=0, wenn M_Row=0 & M_Col=0
    if (RowColCodeLen > 0) {
      Bitstream(SevenOfNine(M_Row, Pow2Int32[RowColCodeLen]));
      //      cout << ",Rows=" << 1 * M_Row << "\n";
      ShowBitstream();
      Bitstream(SevenOfNine(M_Col, Pow2Int32[RowColCodeLen]));
      //      cout << ",Cols=" << 1 * M_Col << "\n";
      ShowBitstream();
      // 3:+-,2:-,1:+,0:no Values
      Bitstream(SevenOfNine(SgnUsed, 4));
      //      cout << ",SgnUsed=" << 1 * SgnUsed << ":"
      //           << ((SgnUsed % 2 == 1) ? "+" : "") << ((SgnUsed > 1) ? "-" :
      //           "")
      //           << "\n";
      ShowBitstream();
      if (SgnUsed > 0) {
        // Kann Werte annehmen: 0..2^31-1, Option Base 0
        Bitstream(SevenOfNine(M_NonZeroSize, M_Row * M_Col + 1));
        //        cout << ",NonZeroSize=" << M_NonZeroSize << "\n";
        ShowBitstream();
        // 4 Bit für Fragment 0..14: Len = 7+(0..14), 15: Len=23
        Bitstream(
            SevenOfNine(((FragLen < 22) ? (FragLen - 7) : (FragLen - 8)), 16));
        //        cout << ",FragLen=" << 1 * FragLen << "\n";
        ShowBitstream();
        // Kann Werte annehmen: 0..7, Option Base 0
        Bitstream(SevenOfNine(Pool[2].CodeLen[0], 8));
        //        cout << ",ExpJump1stCodeLen=" << 1 * Pool[2].CodeLen[0] <<
        //        "\n";
        ShowBitstream();
        // Keine Nullen, wenn Matrix voll besetzt
        ZerosAppear = (M_NonZeroSize < M_Row * M_Col + 1);
        if (ZerosAppear) {
          // Kann Werte annehmen: 0..31, Option Base 0
          Bitstream(SevenOfNine(Pool[3].CodeLen[0], 32));
          //          cout << ",Zeros1stCodeLen=" << 1 * Pool[3].CodeLen[0] <<
          //          "\n";
          ShowBitstream();
        }
      }
      for (PNr = 2; PNr < 4; PNr++) {
        // Kein CodeLen-Skipping (keine Matrizen habe kurze Codes..)
        FreeCodes = 1;  // Setze Bereich auf 100% frei
        LastDefinedCode = -1;
        LastDefinedPool = -1;
        LastDefinedCodeLen = Pool[PNr].CodeLen[0] - 1;
        if (LastDefinedCodeLen >= 0) {
          //          cout <<
          //          "------------------------------------------------------------"
          //                  "--\n";
          //          cout << "Pooldaten(" << PNr - 2 << ")\n\n";
          for (t = 0; t < Pool[PNr].PoolsCount; t++) {
            // Ausgabe, wenn eine Codelänge keine Einträge hat
            while (LastDefinedCodeLen < Pool[PNr].CodeLen[t] - 1) {
              LastDefinedCodeLen++;
              MaxPossCode =
                  LastDefinedCode + FreeCodes * Pow2Dbl129[LastDefinedCodeLen];
              MaxPossPool = findPoolNr(1, MaxPossCode);
              Bitstream(SevenOfNine(0, MaxPossPool - LastDefinedPool + 1));
              //              cout << ",Min/MaxPossPool: " << LastDefinedPool <<
              //              "/"
              //                   << 1 * MaxPossPool << "\n";
              ShowBitstream();
            }
            MaxPossCode =
                LastDefinedCode + FreeCodes * Pow2Dbl129[Pool[PNr].CodeLen[t]];
            MaxPossPool = findPoolNr(1, MaxPossCode);
            Bitstream(SevenOfNine(Pool[PNr].LastPoolNr[t] - LastDefinedPool,
                                  MaxPossPool - LastDefinedPool + 1));
            //            cout << ",Min/MaxPossPool: " << LastDefinedPool << "/"
            //                 << 1 * MaxPossPool << "\n";
            ShowBitstream();
            FreeCodes -= (Pool[PNr].Last[t] - LastDefinedCode) /
                         Pow2Dbl129[Pool[PNr].CodeLen[t]];
            // cout<<"FreeCodes: "<<100*FreeCodes<<"%\n";
            LastDefinedPool = Pool[PNr].LastPoolNr[t];
            LastDefinedCode = Pool[PNr - 2].Last[LastDefinedPool];
            LastDefinedCodeLen = Pool[PNr].CodeLen[t];
          }
        }
      }
      //      cout
      //          <<
      //          "--------------------------------------------------------------\n";
      //      cout << "Timeline\n\n";
      FragCodes = Pow2Int32[(SgnUsed == 3) ? FragLen + 1 : FragLen];
      for (uint32_t i = 0; i < TimelineLen; i++) {
        if (ZerosAppear) {
          Bitstream(SevenOfNine(
              ZeroCount[i] + Pool[3].CodeOffset[findPoolNr(3, ZeroCount[i])],
              Pow2Int32[Pool[3].CodeLen[findPoolNr(3, ZeroCount[i])]]));
          //          cout << ZeroCount[i] << " Nullen: "
          //               << ZeroCount[i] + Pool[3].CodeOffset[findPoolNr(3,
          //               ZeroCount[i])]
          //               << ", Len:" << 1 * Pool[3].CodeLen[findPoolNr(3,
          //               ZeroCount[i])]
          //               << "\n";
          ShowBitstream();
        }
        // Liegen alle Zahlen in einer 2-er-Potenz(z.B. Bilddaten) => keine
        // Ausgabe
        if (Pool[2].CodeLen[0] > 0) {
          Bitstream(SevenOfNine(
              ExpJump[i] + Pool[2].CodeOffset[findPoolNr(2, ExpJump[i])],
              Pow2Int32[Pool[2].CodeLen[findPoolNr(2, ExpJump[i])]]));
          //          cout << 1 * ExpJump[i] << " ExpJump: "
          //               << ExpJump[i] + Pool[2].CodeOffset[findPoolNr(2,
          //               ExpJump[i])]
          //               << ", Len:" << 1 * Pool[2].CodeLen[findPoolNr(2,
          //               ExpJump[i])]
          //               << "\n";
          ShowBitstream();
        }
        Bitstream(SevenOfNine(Fragment[i] % FragCodes, FragCodes));
        //        cout << ",Fragment[" << i << "]=" << Fragment[i] % FragCodes
        //        << "\n";
        ShowBitstream();
        //        cout << "\n";
      }
    }

    /**
     * End of original code snippet. Don't change it.
     */

    //    cout << std::endl;
    *blob = std::vector(OutData.begin(), OutData.begin() + OutBytes);
    return 1;
  }

  bool Decompress(const std::vector<uint8_t> &blob,
                  SfCompressor::OriginalData *origin) {
#if 1
    /**
     * Original code Quelletext.cpp:388
     */
    uint8_t RowColCodeLen, SgnUsed, ExpJump1stCodeLen, Zeros1stCodeLen,
        MaxPossPool, FragCodes, LastExpTmp = 127;
    double FreeCodes;  // Variable Auffüllung Codebereich
    bool ZerosAppear;
    int LastDefinedCode, LastDefinedPool, LastDefinedCodeLen;
    uint32_t MaxPossCode, Gelesen;
    uint32_t M_Row, M_Col, M_NonZeroSize, M_RowBased;

    OutBytes = blob.size();
    OutBitsFreeRead = 8;
    OutBitsFree = 0;
    OutByteNr = 0;
    std::copy(blob.begin(), blob.end(), OutData.begin());

    if (FromBitstream(2) == 0) {
      M_RowBased = FromBitstream(1);
      //      cout << "Row-Based: " << 1 * M_RowBased << "\n";
      RowColCodeLen = NineOfSeven(32);
      //      cout << "Row-ColLen: " << 1 * RowColCodeLen << "\n";
      M_Row = NineOfSeven(Pow2Int32[RowColCodeLen]);
      //      cout << "Rows: " << M_Row << "\n";
      M_Col = NineOfSeven(Pow2Int32[RowColCodeLen]);
      //      cout << "Cols: " << M_Col << "\n";
      SgnUsed = NineOfSeven(4);
      //      cout << "SgnUsed: " << 1 * SgnUsed << "\n";
      M_NonZeroSize = NineOfSeven(M_Row * M_Col + 1);
      //      cout << "NonZeroSize: " << M_NonZeroSize << "\n";
      FragLen = NineOfSeven(16);
      FragLen += ((FragLen > 14) ? 8 : 7);
      //      cout << "FragLen: " << 1 * FragLen << "\n";
      ExpJump1stCodeLen = NineOfSeven(8);
      //      cout << "ExpJump1stCodeLen: " << 1 * ExpJump1stCodeLen << "\n";
      // Keine Nullen, wenn Matrix voll besetzt
      ZerosAppear = (M_NonZeroSize < M_Row * M_Col + 1);
      if (ZerosAppear) {
        Zeros1stCodeLen = NineOfSeven(32);
        //        cout << "Zeros1stCodeLen: " << 1 * Zeros1stCodeLen << "\n";
      }
      for (uint8_t PNr = 2; PNr < 4; PNr++) {
        // Kein CodeLen-Skipping (keine Matrizen habe kurze Codes..)
        FreeCodes = 1;  // Setze Bereich auf 100% frei
        LastDefinedCode = -1;
        LastDefinedPool = -1;
        LastDefinedCodeLen =
            (PNr == 2) ? ExpJump1stCodeLen - 1 : Zeros1stCodeLen - 1;
        Pool[PNr].PoolsCount = 0;
        if (LastDefinedCodeLen >= 0) {
          //          cout <<
          //          "------------------------------------------------------------"
          //                  "--\n";
          //          cout << "Pooldaten(" << PNr - 2 << ")\n\n";
          for (int t = 0; t < Pool[PNr - 2].PoolsCount; t++) {
            do {
              LastDefinedCodeLen++;
              MaxPossCode =
                  LastDefinedCode + FreeCodes * Pow2Dbl129[LastDefinedCodeLen];
              MaxPossPool = findPoolNr(1, MaxPossCode);
              Gelesen = NineOfSeven(MaxPossPool - LastDefinedPool + 1);
              //              cout << Gelesen << " " << (MaxPossPool -
              //              LastDefinedPool + 1)
              //                   << " " << 1 * LastDefinedPool << " " << 1 *
              //                   MaxPossPool
              //                   << " "
              //                   << "\n";
            } while (Gelesen == 0);
            Pool[PNr].CodeLen[t] = LastDefinedCodeLen;
            Pool[PNr].LastPoolNr[t] = LastDefinedPool + Gelesen;
            Pool[PNr].Last[t] = Pool[PNr - 2].Last[Pool[PNr].LastPoolNr[t]];
            Pool[PNr].CodeOffset[t] =
                (1 - FreeCodes) * Pow2Int32[LastDefinedCodeLen] -
                LastDefinedCode - 1;
            FreeCodes -= (Pool[PNr].Last[t] - LastDefinedCode) /
                         Pow2Dbl129[Pool[PNr].CodeLen[t]];
            LastDefinedCode = Pool[PNr - 2].Last[Pool[PNr].LastPoolNr[t]];
            LastDefinedPool = LastDefinedPool + Gelesen;
            Pool[PNr].PoolsCount++;
            // cout<<"FreeCodes: "<<100*FreeCodes<<"%\n";
            if (FreeCodes == 0) break;
          }
        }
      }

      *origin = {.frag_length = FragLen,
                 .row_based = static_cast<bool>(M_RowBased),
                 .rows = M_Row,
                 .columns = M_Col,
                 .indexes{},
                 .values{}};
      //      cout
      //          <<
      //          "--------------------------------------------------------------\n";
      //      cout << "Timeline\n\n";
      uint8_t PushLen;
      int ExpJmpTmp, ExpNow = 127, IndexNow = -1;
      double MySign = ((SgnUsed == 1) ? 1.0 : -1.0), MyFragValue;
      FragCodes = Pow2Int32[(SgnUsed == 3) ? FragLen + 1 : FragLen];
      for (uint32_t i = 0; i < M_NonZeroSize; i++) {
        if (ZerosAppear) {
          ZeroCount[i] = FromBitstream(Pool[3].CodeLen[0]);
          if (ZeroCount[i] > Pool[3].Last[0]) {
            for (uint8_t t = 1; t < Pool[3].PoolsCount; t++) {
              PushLen = Pool[3].CodeLen[t] - Pool[3].CodeLen[t - 1];
              ZeroCount[i] = (ZeroCount[i] << PushLen) + FromBitstream(PushLen);
              if (ZeroCount[i] <= Pool[3].Last[t] + Pool[3].CodeOffset[t]) {
                ZeroCount[i] -= Pool[3].CodeOffset[t];
                break;
              }
            }
          }
          IndexNow += 1 + ZeroCount[i];

          origin->indexes.push_back(IndexNow);
          //          cout << IndexNow << " ";
        }
        ExpJmpTmp = FromBitstream(Pool[2].CodeLen[0]);
        if (ExpJmpTmp > Pool[2].Last[0]) {
          for (uint8_t t = 1; t < Pool[2].PoolsCount; t++) {
            PushLen = Pool[2].CodeLen[t] - Pool[2].CodeLen[t - 1];
            ExpJmpTmp = (ExpJmpTmp << PushLen) + FromBitstream(PushLen);
            if (ExpJmpTmp <= Pool[2].Last[t] + Pool[2].CodeOffset[t]) {
              ExpJmpTmp -= Pool[2].CodeOffset[t];
              break;
            }
          }
        }
        ExpJump[i] = ExpJmpTmp;
        // cout<<1*ExpJump[i]<<" ";
        if (LastExpTmp < 128) {
          if (ExpJump[i] <= 2 * LastExpTmp) {
            if (ExpJump[i] % 2 == 0) {
              ExpJump[i] = ExpJump[i] / 2 + LastExpTmp;
            } else {
              ExpJump[i] = LastExpTmp - (ExpJump[i] + 1) / 2;
            }
          }
          // Sonst bleibt ExpJump[i]...
        } else {
          if (ExpJump[i] <= 2 * (255 - LastExpTmp)) {
            if (ExpJump[i] % 2 == 0) {
              ExpJump[i] = ExpJump[i] / 2 + LastExpTmp;
            } else {
              ExpJump[i] = LastExpTmp - (ExpJump[i] + 1) / 2;
            }
          } else {
            ExpJump[i] = 255 - ExpJump[i];
          }
          // Sonst bleibt ExpJump[i]...
        }
        LastExpTmp = ExpJump[i];
        // cout<<1*ExpJump[i]<<" ";
        // Vorzeichen holen, wenn geschrieben
        if (SgnUsed == 3) {
          MySign = ((FromBitstream(1) == 1) ? -1.0 : 1.0);
        }
        MyFragValue =
            MySign * (1.0 + FromBitstream(FragLen) / Pow2Dbl129[FragLen]);
        if (LastExpTmp > 127) {
          MyFragValue *= Pow2Dbl129[LastExpTmp - 127];
        } else {
          if (LastExpTmp > 0) {
            MyFragValue /= Pow2Dbl129[127 - LastExpTmp];
          } else {
            MyFragValue = 0;
          }
        }

        //        cout << MyFragValue << " ";
        //        cout << "\n";
        origin->values.push_back(MyFragValue);
      }

    } else {
      std::cerr << "Falsche Version\n";
      return false;
    }
    /**
     * End of original code snippet. Don't change it.
     */
#endif
    return true;
  }

 private:
  void setPoolSearch(uint8_t PNr) {
    Pool[PNr].BisecSteps = 0;
    if ((Pool[PNr].PoolsCount > 1)) {
      while (Pool[PNr].PoolsCount > Pow2Int32[Pool[PNr].BisecSteps + 1]) {
        Pool[PNr].BisecStep[Pool[PNr].BisecSteps] =
            Pow2Int32[Pool[PNr].BisecSteps];
        Pool[PNr].BisecSteps++;
      }
      Pool[PNr].BisecStep[Pool[PNr].BisecSteps] =
          Pool[PNr].PoolsCount - Pow2Int32[Pool[PNr].BisecSteps];
      Pool[PNr].BisecSteps++;
    }
  }

  uint8_t findPoolNr(uint8_t PNr, uint32_t TestNr) {
    uint8_t PoolNrTmp = 0, TestPoolNrTmp;  // Zwischenwerte f_r Poolzuordnung
    for (int u = Pool[PNr].BisecSteps - 1; u >= 0; u--) {
      TestPoolNrTmp = Pool[PNr].BisecStep[u] + PoolNrTmp;
      if (TestNr > Pool[PNr].Last[TestPoolNrTmp - 1]) {
        PoolNrTmp = TestPoolNrTmp;
      }
    }
    return PoolNrTmp;
  }

  // Setzt Suchschritte f_r reverse Bisektion _hne berlauf
  void ShowPoolSearch(uint8_t PNr, bool ShowUnused) {
    //    cout << "Bisection(" << 1 * PNr << "): ";
    //    for (int t = Pool[PNr].BisecSteps - 1; t >= 0; t--) {
    //      cout << 1 * Pool[PNr].BisecStep[t] << " ";
    //    }
    //    cout << "\n";
    //    for (uint32_t t = 0; t < ((ShowUnused == true) ? Pool[PNr].PoolsCount
    //                                                   : Pool[PNr].PoolsUsed);
    //         t++) {
    //      cout << "P " << 1 * PNr << "." << ((t < 10) ? "0" : "") << t << "
    //      Last: "; for (uint32_t u = 1000000000; u > 9; u /= 10) {
    //        if (Pool[PNr].Last[t] < u) cout << " ";
    //      }
    //      cout << Pool[PNr].Last[t] << " Size: ";
    //      for (uint32_t u = 100000000; u > 9; u /= 10) {
    //        if (Pool[PNr].CodeQty[t] < u) cout << " ";
    //      }
    //      cout << Pool[PNr].CodeQty[t] << " DS: ";
    //      // for(unsigned __int32
    //      // u=100000000;u>9;u/=10){if(Pool[PNr].DSQty[t]<u)cout<<" ";}
    //      cout << Pool[PNr].CodeOffset[t] << " Len: ";
    //      if (Pool[PNr].CodeLen[t] < 10) cout << " ";
    //      cout << 1 * Pool[PNr].CodeLen[t] << " LastPoolNr: ";
    //      if (Pool[PNr].LastPoolNr[t] < 10) cout << " ";
    //      cout << 1 * Pool[PNr].LastPoolNr[t] << "\n";
    //    }
    //    cout << "\n";
  }

  typeOutAdder SevenOfNine(uint32_t NrSel, uint32_t NrPoss) {
    typeOutAdder AusgabeCode;
    AusgabeCode.BitsUsed = findPoolNr(4, NrPoss);
    AusgabeCode.BitsOut = NrSel;
    uint32_t MaxMampfen = Pow2Int32[AusgabeCode.BitsUsed] - NrPoss + 1;
    uint32_t Pow2ForZero;
    // Berechne max. n�tige Bits
    if (MaxMampfen > 1) {
      Pow2ForZero = findPoolNr(5, MaxMampfen);
      if (NrSel > 0) {
        if (NrSel <= MaxMampfen - Pow2Int32[Pow2ForZero]) {
          AusgabeCode.BitsOut += Pow2Int32[Pow2ForZero - 1] - 1;
          AusgabeCode.BitsUsed--;
        } else {
          AusgabeCode.BitsOut += MaxMampfen - 1;
        }
      } else {
        AusgabeCode.BitsUsed -= Pow2ForZero;
      }
    }
    //    cout << ((NrSel < 10) ? " " : "") << NrSel << "/0-"
    //         << ((NrPoss - 1 < 10) ? " " : "") << NrPoss - 1
    //         << ",Code: " << ((AusgabeCode.BitsOut < 10) ? " " : "")
    //         << AusgabeCode.BitsOut << "(" << 1 * AusgabeCode.BitsUsed << "/"
    //         << 1 * findPoolNr(4, NrPoss) << "Bit)";
    return AusgabeCode;
  }

  uint32_t NineOfSeven(uint32_t NrPoss) {
    uint32_t NrSel;
    uint8_t BitsMax = findPoolNr(4, NrPoss);
    uint32_t MaxMampfen = Pow2Int32[BitsMax] - NrPoss + 1;
    uint32_t Pow2ForZero;
    // Berechne max. n_tige Bits
    if (MaxMampfen == 1) {
      NrSel = FromBitstream(BitsMax);
    } else {
      // Wie viele (2-er Pot) Codes f_r die Null??
      Pow2ForZero = findPoolNr(5, MaxMampfen);
      NrSel = FromBitstream(BitsMax - Pow2ForZero);
      if (NrSel > 0) {
        NrSel = (NrSel << (Pow2ForZero - 1)) + FromBitstream(Pow2ForZero - 1);
        if (NrSel + 1 - Pow2Int32[Pow2ForZero - 1] <=
            MaxMampfen - Pow2Int32[Pow2ForZero]) {
          NrSel = NrSel + 1 - Pow2Int32[Pow2ForZero - 1];
        } else {
          NrSel = (NrSel << 1) + FromBitstream(1) - MaxMampfen + 1;
        }
      }
    }
    return NrSel;
  }

  void ShowBitstream(uint32_t OutBytes) {
    //    uint8_t MyOutput;
    //    cout << "OUT: ";
    //    for (int t = 0; t < OutBytes; t++) {
    //      MyOutput = OutData[t];
    //      if (t + 1 < OutBytes) {
    //        for (int u = 0; u < 8; u++) {
    //          cout << (MyOutput > 127) ? "1" : "0";
    //          MyOutput = (MyOutput << 1);
    //        }
    //      } else {
    //        for (int u = 0; u < 8 - OutBitsFree; u++) {
    //          cout << (MyOutput > 127) ? "1" : "0";
    //          MyOutput = (MyOutput << 1);
    //        }
    //      }
    //      cout << " ";
    //    }
    //    cout << "\n";
  }

  void Bitstream() {
    // wenn Codel�nge Null => nichts machen
    while (AusgabeCode.BitsUsed > 0) {
      // Schaffe Platz (dann 1-8Bit frei)
      if (OutBitsFree == 0) {
        OutData[OutBytes] = 0;  // hier l�schen!!
        OutBytes++;
        OutBitsFree = 8;
      }
      if (AusgabeCode.BitsUsed <= OutBitsFree) {
        // Code kann komplett eingebracht werden
        OutBitsFree -= AusgabeCode.BitsUsed;
        OutData[OutBytes - 1] += (AusgabeCode.BitsOut << OutBitsFree);
        break;
      } else {
        // Codeschnipsel trennen & einbringen
        OutData[OutBytes - 1] +=
            (AusgabeCode.BitsOut >> (AusgabeCode.BitsUsed - OutBitsFree));
        AusgabeCode.BitsUsed -= OutBitsFree;
        AusgabeCode.BitsOut =
            AusgabeCode.BitsOut % Pow2Int32[AusgabeCode.BitsUsed];
        OutBitsFree = 0;
      }
    }
  }

  void ShowBitstream() {
    //    uint8_t MyOutput;
    //    cout << "OUT: ";
    //    for (int t = 0; t < OutBytes; t++) {
    //      MyOutput = OutData[t];
    //      if (t + 1 < OutBytes) {
    //        for (int u = 0; u < 8; u++) {
    //          cout << (MyOutput > 127) ? "1" : "0";
    //          MyOutput = (MyOutput << 1);
    //        }
    //      } else {
    //        for (int u = 0; u < 8 - OutBitsFree; u++) {
    //          cout << (MyOutput > 127) ? "1" : "0";
    //          MyOutput = (MyOutput << 1);
    //        }
    //      }
    //      cout << " ";
    //    }
    //    cout << "\n";
  }

  void Bitstream(typeOutAdder AusgabeCode) {
    // wenn Codel�nge Null => nichts machen
    while (AusgabeCode.BitsUsed > 0) {
      // Schaffe Platz (dann 1-8Bit frei)
      if (OutBitsFree == 0) {
        OutData[OutBytes] = 0;  // hier l�schen!!
        OutBytes++;
        OutBitsFree = 8;
      }
      if (AusgabeCode.BitsUsed <= OutBitsFree) {
        // Code kann komplett eingebracht werden
        OutBitsFree -= AusgabeCode.BitsUsed;
        OutData[OutBytes - 1] += (AusgabeCode.BitsOut << OutBitsFree);
        break;
      } else {
        // Codeschnipsel trennen & einbringen
        OutData[OutBytes - 1] +=
            (AusgabeCode.BitsOut >> (AusgabeCode.BitsUsed - OutBitsFree));
        AusgabeCode.BitsUsed -= OutBitsFree;
        AusgabeCode.BitsOut =
            AusgabeCode.BitsOut % Pow2Int32[AusgabeCode.BitsUsed];
        OutBitsFree = 0;
      }
    }
  }

  bfloat16 float_to_bfloat16(float TestFloat) {
    bfloat16 New_bf16;
    New_bf16.Exp = 0;
    New_bf16.SgnFrag = 0;
    // Shortcut for 0 with Sgn Shift to +
    if (TestFloat == 0) {
      return New_bf16;
    }
    // 64-Bit Adress of float => 32 Bit unsigned int
    uint64_t *adresse = reinterpret_cast<uint64_t *>(&TestFloat);
    uint32_t Zwischen = *adresse;
    // Takes the 8 Bit of the Exponent
    New_bf16.Exp = ((Zwischen << 1) >> 24);
    // Takes the left FragLen Bit of the Fragment
    New_bf16.SgnFrag = ((Zwischen << 9) >> (32 - FragLen));
    // Rounding of Fragment with 8th Bit of Fragment
    if ((FragLen < 23) && ((Zwischen << (9 + FragLen)) >> 31) == 1) {
      // Increment Fragment
      New_bf16.SgnFrag++;
      // Test if Overflow of Fragment
      if (New_bf16.SgnFrag == Pow2Int32[FragLen]) {
        // Reset Overflow
        New_bf16.SgnFrag = 0;
        // Increment of Exponent, if not Inf/NaN (may cause Inf..)
        if (New_bf16.Exp < 255) {
          New_bf16.Exp++;
        }
      }
    }
    // Sgn as leeding Bit of Fragment (0: allways +)
    New_bf16.Sgn = (Zwischen >> 31);
    New_bf16.SgnFrag = New_bf16.SgnFrag + (New_bf16.Sgn << FragLen);
    return New_bf16;
  }

  uint32_t FromBitstream(uint8_t BitCount) {
    uint32_t ResultBits = 0;
    uint8_t RestBits = BitCount;
    // wenn Codel�nge Null => nichts machen
    while (RestBits > 0) {
      // Schaffe Platz (dann 1-8Bit frei)
      if (OutBitsFreeRead == 0) {
        OutByteNr++;
        OutBitsFreeRead = 8;
      }
      if (RestBits <= OutBitsFreeRead) {
        // Code kann komplett geholt werden
        ResultBits =
            (ResultBits << RestBits) + (OutData[OutByteNr] >> (8 - RestBits));
        OutData[OutByteNr] = OutData[OutByteNr] << RestBits;
        OutBitsFreeRead -= RestBits;
        break;
      } else {
        // Codeschnipsel trennen & einbringen
        ResultBits = (ResultBits << OutBitsFreeRead) +
                     (OutData[OutByteNr] >> (8 - OutBitsFreeRead));
        RestBits -= OutBitsFreeRead;
        OutBitsFreeRead = 0;
      }
    }
    return ResultBits;
  }

  std::array<uint32_t, MAXCODELEN> Pow2Int32{};  // Pow2 shortcuts
  std::array<double, 129> Pow2Dbl129{};
  std::array<typePool, 6> Pool{};
  typeOutAdder AusgabeCode{};

  // output
  uint8_t OutBitsFree = 0;
  std::vector<uint8_t> OutData{};
  uint32_t OutBytes = 0;
  uint8_t FragLen = 0;
  uint8_t OutBitsFreeRead = 8;
  uint32_t OutByteNr = 0;

  std::vector<uint32_t> ZeroCount{};
  std::vector<uint32_t> Fragment{};
  std::vector<uint8_t> ExpJump{};
};

SfCompressor::SfCompressor(size_t buffer_size)
    : impl_(std::make_unique<Impl>(buffer_size)) {}
SfCompressor::~SfCompressor() = default;

bool SfCompressor::Compress(const SfCompressor::OriginalData &origin,
                            std::vector<uint8_t> *blob) const {
  return impl_->Compress(origin, blob);
}

bool SfCompressor::Decompress(const std::vector<uint8_t> &blob,
                              SfCompressor::OriginalData *origin) const {
  return impl_->Decompress(blob, origin);
}

bool SfCompressor::OriginalData::operator==(
    const SfCompressor::OriginalData &rhs) const {
  bool without_values =
      std::tie(frag_length, row_based, rows, columns, indexes) ==
      std::tie(rhs.frag_length, rhs.row_based, rhs.rows, rhs.columns,
               rhs.indexes);
  if (without_values && values.size() == rhs.values.size()) {
    const auto error = 1.0 / (std::pow(2, frag_length + 1) - 2);
    for (int i = 0; i < values.size(); ++i) {
      if (std::abs((values[i] - rhs.values[i]) / values[i]) > error) {
        return false;
      }
    }
    return true;
  }

  return false;
}

// bool SfCompressor::OriginalData::operator==(
//     const SfCompressor::OriginalData &rhs) const {
//   return std::tie(frag_length, row_based, rows, columns, indexes, values) ==
//          std::tie(rhs.frag_length, rhs.row_based, rhs.rows, rhs.columns,
//                   rhs.indexes, rhs.values);
// }

bool SfCompressor::OriginalData::operator!=(
    const SfCompressor::OriginalData &rhs) const {
  return !(rhs == *this);
}
}  // namespace drift::wavelet::internal
