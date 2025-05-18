import React, { useState, useEffect, useContext } from 'react';
import { useLocation, useNavigate } from 'react-router-dom';
import { UserContext } from '../UserContext';

const Tictac = () => {
  const { state } = useLocation();
  const { sessionId } = state;           // Passed from lobby
  const { username } = useContext(UserContext);
  const navigate = useNavigate();

  // Server‑driven state
  const [board, setBoard]               = useState([
    ['', '', ''],
    ['', '', ''],
    ['', '', ''],
  ]);
  const [currentPlayer, setCurrentPlayer] = useState(null);
  const [winner, setWinner]               = useState(null);
  const [ws, setWs]                       = useState(null);

  // ====== 1. Establish WebSocket and handlers ======
  useEffect(() => {
    const socket = new WebSocket(`ws://${window.location.host}/ws/game/${sessionId}`);

    socket.onopen = () => {
      console.log('WebSocket connected to session', sessionId);
    };

  socket.onmessage = (event) => {
    const state = JSON.parse(event.data);
    setBoard(state.board);
    setCurrentPlayer(state.currentPlayer);
    setWinner(state.winner || null);

    if (state.finished) {
      // You could disable further clicks, show rematch button, etc.
      setIsFinished(true);
    }
  };


    socket.onclose = () => {
      console.log('WebSocket closed');
    };

    setWs(socket);
    return () => socket.close();
  }, [sessionId]);

  // ====== 2. Handle user clicks & send moves ======
  const handleClick = (row, col) => {
    if (!ws || winner) return;                 // no socket or already finished
    if (board[row][col] !== ''                 // occupied
        || currentPlayer !== username)         // not your turn
      return;

    // Send only the move payload; server knows sessionId from URL
    ws.send(JSON.stringify({ username, row, col }));
  };

  // ====== 3. If game over, optionally allow “Play Again” or redirect ======
  const handleRestart = () => {
    // Could call a server‑side reset endpoint here; for now, back to lobby
    navigate('/battle_lobby', { state: { gameType: 'tic', username } });
  };

  // ====== 4. Render ======
  const renderCell = (r, c) => {
    return (
      <div
        key={`${r}-${c}`}
        style={{
          ...styles.cell,
          cursor: (board[r][c] === '' && currentPlayer === username && !winner)
            ? 'pointer' : 'not-allowed',
        }}
        onClick={() => handleClick(r, c)}
      >
        {board[r][c]}
      </div>
    );
  };

  return (
    <div style={styles.container}>
      <h1 style={styles.heading}>🎮 Tic Tac Toe</h1>
      <p style={styles.status}>
        {winner
          ? `🏆 Winner: ${winner}`
          : board.flat().every(cell => cell !== '')
          ? "🤝 It's a Tie!"
          : currentPlayer === username
          ? 'Your move'
          : `${currentPlayer || '...'}'s move`}
      </p>

      <div style={styles.board}>
        {board.map((rowArr, r) =>
          rowArr.map((_, c) => renderCell(r, c))
        )}
      </div>

      {winner || board.flat().every(cell => cell !== '') ? (
        <button onClick={handleRestart} style={styles.resetButton}>
          Play Again
        </button>
      ) : null}
    </div>
  );
};

const styles = {
  container: {
    textAlign: 'center',
    marginTop: '60px',
    fontFamily: 'Arial, sans-serif',
    color: '#2c3e50',
  },
  heading: {
    fontSize: '2.5rem',
    marginBottom: '10px',
  },
  status: {
    fontSize: '1.4rem',
    marginBottom: '20px',
  },
  board: {
    display: 'grid',
    gridTemplateColumns: 'repeat(3, 100px)',
    gridGap: '10px',
    justifyContent: 'center',
    marginBottom: '20px',
  },
  cell: {
    width: '100px',
    height: '100px',
    backgroundColor: '#ecf0f1',
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    fontSize: '2rem',
    borderRadius: '10px',
    boxShadow: '0 2px 5px rgba(0,0,0,0.2)',
    transition: '0.2s',
  },
  resetButton: {
    padding: '10px 20px',
    fontSize: '1rem',
    borderRadius: '8px',
    backgroundColor: '#3498db',
    color: 'white',
    border: 'none',
    cursor: 'pointer',
  },
};

export default Tictac;
